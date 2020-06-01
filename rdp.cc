#include "rdp.h"
#include "generator.h"
#include "channels/pointer.h"
#include "channels/channel.h"
#include "channels/clipboard.h"

SessionData** sessions;
int sessionCount = 0;
int add_session(SessionData* session)
{
	if (sessions == NULL) {
		sessionCount = 1;
		sessions = (SessionData **)malloc(sizeof(SessionData *));
	}
	else {
		sessionCount += 1;
		SessionData **newSessions = (SessionData **)malloc(sizeof(SessionData *) * sessionCount);
		memcpy(newSessions, sessions, sizeof(SessionData *) * (sessionCount - 1));
		free(sessions);
		sessions = newSessions;
	}

	sessions[sessionCount - 1] = session;
	return sessionCount - 1;
}

static BOOL nodefreerdp_client_global_init(void){
	WSADATA wsaData;
	WSAStartup(0x101, &wsaData);
	return TRUE;
}

static void nodefreerdp_client_global_uninit(void){
	WSACleanup();
}


static BOOL nodefreerdp_pre_connect(freerdp* instance)
{
	rdpSettings* settings;
	if (!instance || !instance->context || !instance->settings)
		return FALSE;

	settings = instance->settings;
	settings->OsMajorType = OSMAJORTYPE_WINDOWS;
	settings->OsMinorType = OSMINORTYPE_WINDOWS_NT;
	settings->OrderSupport[NEG_DSTBLT_INDEX] = TRUE;
	settings->OrderSupport[NEG_PATBLT_INDEX] = TRUE;
	settings->OrderSupport[NEG_SCRBLT_INDEX] = TRUE;
	settings->OrderSupport[NEG_OPAQUE_RECT_INDEX] = TRUE;
	settings->OrderSupport[NEG_DRAWNINEGRID_INDEX] = FALSE;
	settings->OrderSupport[NEG_MULTIDSTBLT_INDEX] = FALSE;
	settings->OrderSupport[NEG_MULTIPATBLT_INDEX] = FALSE;
	settings->OrderSupport[NEG_MULTISCRBLT_INDEX] = FALSE;
	settings->OrderSupport[NEG_MULTIOPAQUERECT_INDEX] = TRUE;
	settings->OrderSupport[NEG_MULTI_DRAWNINEGRID_INDEX] = FALSE;
	settings->OrderSupport[NEG_LINETO_INDEX] = TRUE;
	settings->OrderSupport[NEG_POLYLINE_INDEX] = TRUE;
	settings->OrderSupport[NEG_MEMBLT_INDEX] = settings->BitmapCacheEnabled;
	settings->OrderSupport[NEG_MEM3BLT_INDEX] = settings->BitmapCacheEnabled;
	settings->OrderSupport[NEG_MEMBLT_V2_INDEX] = settings->BitmapCacheEnabled;
	settings->OrderSupport[NEG_MEM3BLT_V2_INDEX] = settings->BitmapCacheEnabled;
	settings->OrderSupport[NEG_SAVEBITMAP_INDEX] = FALSE;
	settings->OrderSupport[NEG_GLYPH_INDEX_INDEX] = TRUE;
	settings->OrderSupport[NEG_FAST_INDEX_INDEX] = TRUE;
	settings->OrderSupport[NEG_FAST_GLYPH_INDEX] = TRUE;
	settings->OrderSupport[NEG_POLYGON_SC_INDEX] = TRUE;
	settings->OrderSupport[NEG_POLYGON_CB_INDEX] = TRUE;
	settings->OrderSupport[NEG_ELLIPSE_SC_INDEX] = FALSE;
	settings->OrderSupport[NEG_ELLIPSE_CB_INDEX] = FALSE;
	if (!freerdp_client_load_addins(instance->context->channels, instance->settings))
		return -1;
	PubSub_SubscribeChannelConnected(instance->context->pubSub,node_OnChannelConnectedEventHandler);
	PubSub_SubscribeChannelDisconnected(instance->context->pubSub,node_OnChannelDisconnectedEventHandler);

	return TRUE;
}

BOOL nodefreerdp_begin_paint(rdpContext* context)
{
	rdpGdi* gdi = context->gdi;
	gdi->primary->hdc->hwnd->invalid->null = TRUE;
	return TRUE;
}
static BOOL nodefreerdp_end_paint(rdpContext* context)
{
	nodeContext *nc = (nodeContext*)context;
	rdpGdi* gdi = context->gdi;
	draw_args *args = new draw_args;
	args->bpp = 4;
	if (nc->keyframe) {
		args->x = 0;
		args->y = 0;
		args->w = gdi->width;
		args->h = gdi->height;
		int size = args->w * args->h * args->bpp;
		args->buffer = new BYTE[size];
		memcpy(args->buffer, gdi->primary_buffer, size);
		nc->keyframe = false;
	}
	else {
		if (gdi->primary->hdc->hwnd->invalid->null)
			return TRUE;
		int ninvalid = gdi->primary->hdc->hwnd->ninvalid;
		if (ninvalid < 1)
			return TRUE;

		args->x = gdi->primary->hdc->hwnd->invalid->x;
		args->y = gdi->primary->hdc->hwnd->invalid->y;
		args->w = gdi->primary->hdc->hwnd->invalid->w;
		args->h = gdi->primary->hdc->hwnd->invalid->h;
		int size = args->w * args->h * args->bpp;
		args->buffer = new BYTE[size];
		int dest_pos = 0;
		int dest_line_width = args->w * args->bpp;
		for (int i = args->y; i < args->y + args->h; i++) {
			// memcopy only columns that are relevant
			int start_pos = (i * gdi->width * args->bpp) + (args->x * args->bpp);
			BYTE* src = &gdi->primary_buffer[start_pos];
			BYTE* dest = &args->buffer[dest_pos];
			memcpy(dest, src, dest_line_width);
			dest_pos += dest_line_width;
		}
	}
	generator_emit(nc->generatorContext, &DRAW_GENERATOR_TYPE, args);
	return TRUE;
}


static BOOL nodefreerdp_post_connect(freerdp* instance)
{
	if (!gdi_init(instance, PIXEL_FORMAT_RGBX32))
		return FALSE;
	instance->update->BeginPaint = nodefreerdp_begin_paint;
	instance->update->EndPaint = nodefreerdp_end_paint;

	nodeContext *nContext = (nodeContext*)instance->context;
	connect_args *args = (connect_args *)malloc(sizeof(connect_args));
	generator_emit(nContext->generatorContext, &CONNECT_GENERATOR_TYPE, args);

	return TRUE;
}

static BOOL nodefreerdp_client_new(freerdp* instance, rdpContext* context)
{
	if (!(nodefreerdp_client_global_init()))
		return FALSE;

	instance->PreConnect = nodefreerdp_pre_connect;
	instance->PostConnect = nodefreerdp_post_connect;
	return TRUE;
}

static DWORD WINAPI node_client_thread(LPVOID lpParam)
{
	close_args *args = (close_args *)malloc(sizeof(close_args));
	nodeContext * nContext = (nodeContext *)lpParam;
	rdpContext * context = (rdpContext *)lpParam;
	DWORD nCount;
	HANDLE handles[64];
	args->msg = "unknowing error";
	if (!freerdp_connect(context->instance))
	{
		args->msg = "connection failure";
		WLog_ERR(TAG, "connection failure");
		goto error;
	}
	while (1)
	{
		nCount = 0;

		{
			DWORD tmp = freerdp_get_event_handles(context, &handles[nCount], 64 - nCount);

			if (tmp == 0)
			{
				WLog_ERR(TAG, "freerdp_get_event_handles failed");
				break;
			}

			nCount += tmp;
		}
		if (MsgWaitForMultipleObjects(nCount, handles, FALSE, 1000,
			QS_ALLINPUT) == WAIT_FAILED)
		{
			WLog_ERR(TAG, "wfreerdp_run: WaitForMultipleObjects failed: 0x%08lX",
				GetLastError());
			break;
		}
		{
			if (!freerdp_check_event_handles(context))
			{
				if (client_auto_reconnect(context->instance))
					continue;

				WLog_ERR(TAG, "Failed to check FreeRDP file descriptor");
				break;
			}
		}
		if (freerdp_shall_disconnect(context->instance))
			break;

		if( nContext->session->stopping ){
			args->msg = "session stopped";
			break;
		}
	}
error:
	freerdp_free(context->instance);
	generator_emit(nContext->generatorContext, &CLOSE_GENERATOR_TYPE, args);
	ExitThread(0);
	return 0;
}


static int nodefreerdp_client_start(rdpContext* context)
{
	HWND hWndParent;
	HINSTANCE hInstance;
	
	HANDLE thread_id = CreateThread(NULL, 0, node_client_thread, (void*)context, 0,NULL);

	return 0;
}

static int nodefreerdp_client_stop(rdpContext* context)
{
	

	return 0;
}



int RdpClientEntry(RDP_CLIENT_ENTRY_POINTS* pEntryPoints){
	pEntryPoints->Version = 1;
	pEntryPoints->Size = sizeof(RDP_CLIENT_ENTRY_POINTS_V1);
	pEntryPoints->GlobalInit = nodefreerdp_client_global_init;
	pEntryPoints->GlobalUninit = nodefreerdp_client_global_uninit;
	pEntryPoints->ContextSize = sizeof(nodeContext);
	pEntryPoints->ClientNew = nodefreerdp_client_new;
	pEntryPoints->ClientStart = nodefreerdp_client_start;
	pEntryPoints->ClientStop = nodefreerdp_client_stop;
	return 0;
}

//entry
int node_freerdp_connect(int argc, char* argv[], Callback *callback)
{
	int status;
	nodeContext* nContext;
	rdpContext* context;
	rdpSettings* settings;
	RDP_CLIENT_ENTRY_POINTS clientEntryPoints;
	ZeroMemory(&clientEntryPoints, sizeof(RDP_CLIENT_ENTRY_POINTS));
	clientEntryPoints.Size = sizeof(RDP_CLIENT_ENTRY_POINTS);
	clientEntryPoints.Version = RDP_CLIENT_INTERFACE_VERSION;
	RdpClientEntry(&clientEntryPoints);
	//init freerdp context
	context = freerdp_client_context_new(&clientEntryPoints);
	nContext = (nodeContext *)context;
	nContext->generatorContext = new GeneratorContext;
	nContext->generatorContext->callback = callback;
	nContext->keyframe = false;
	settings = context->settings;
	status = freerdp_client_settings_parse_command_line(settings, argc,argv, FALSE);
	if (status)
	{
		freerdp_client_settings_command_line_status_print(settings, status, argc, argv);
		return -1;
	}

	//start freerdp client
	freerdp_client_start(context);
	node_register_pointer(context->graphics);

	SessionData * session = (SessionData*) malloc(sizeof(SessionData));
	ZeroMemory(session, sizeof(sizeof(SessionData)));
	session->stopping = false;
	session->instance = context->instance;
	int index = add_session(session);
	nContext->session = session;
	return index;

}


void node_freerdp_close(int session_index)
{
  // NOTE: Doesn't block on closed session, will send closed event when completed
  SessionData *session = sessions[session_index];
  session->stopping = true;
}

void node_freerdp_send_key_event_scancode(int session_index, int code, int pressed)
{
  SessionData* session = sessions[session_index];
  freerdp* instance = session->instance;
  rdpInput* input = instance->input;
  freerdp_input_send_keyboard_event_ex(input, pressed, code);
}

void node_freerdp_send_pointer_event(int session_index, int flags, int x, int y)
{
  SessionData* session = sessions[session_index];
  freerdp* instance = session->instance;
  rdpInput* input = instance->input;

  freerdp_input_send_mouse_event(input, flags, x, y);
}

void node_freerdp_cliprdr_set_data(int session_index,byte* clipboardData,int length) {
	SessionData* session = sessions[session_index];
	freerdp* instance = session->instance;
	nodeContext * nContext = (nodeContext*)instance->context;
	nContext->clipboard->length = length;
	nContext->clipboard->buffer = clipboardData;
	cliprdr_send_format_list(nContext->clipboardContext);
}


void node_freerdp_request_keyframe(int session_index)
{
  // NOTE: Doesn't block on closed session, will send closed event when completed
  SessionData *session = sessions[session_index];
  nodeContext * nContext = (nodeContext*)session->instance->context;
  nContext->keyframe = true;
  //freerdp_disconnect(session->instance);
}