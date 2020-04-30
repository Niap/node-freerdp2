

#include <winsock2.h>
#include <Windows.h>
#include <ws2tcpip.h>

#include "rdp.h"
#include "generator.h"
#include "context.h"
#include <stdio.h>

#include <freerdp/freerdp.h>
#include <freerdp/constants.h>
#include <freerdp/log.h>
#include <freerdp/gdi/gdi.h>
#include <freerdp/client/cmdline.h>
#include <freerdp/client/channels.h>
#include <freerdp/channels/channels.h>

#include <winpr/crt.h>
#include <winpr/synch.h>

#include <nan.h>

using Nan::Callback;

using v8::Object;
using v8::Array;
using v8::Number;
using v8::Value;
using v8::Local;
using v8::String;
using Nan::New;
using Nan::Null;


struct connect_args {};

Local<Array> connect_args_parser(void *generic) {
  connect_args *args = static_cast<connect_args *>(generic);
  Local<Array> argv = New<Array>();
  free(args);
  return argv;
}
struct close_args {};

Local<Array> close_args_parser(void *generic) {
  connect_args *args = static_cast<connect_args *>(generic);
  Local<Array> argv = New<Array>();
  free(args);
  return argv;
}

const struct GeneratorType CONNECT_GENERATOR_TYPE = {"connect",connect_args_parser};
const struct GeneratorType CLOSE_GENERATOR_TYPE = {"close",close_args_parser };

struct tf_context
{
	rdpContext _p;
};
typedef struct tf_context tfContext;

struct thread_data
{
  freerdp* instance;
  bool stopping;
};

thread_data** sessions;
int sessionCount = 0;

int add_session(thread_data* session)
{
  if(sessions == NULL) {
    sessionCount = 1;
    sessions = (thread_data **)malloc(sizeof(thread_data *));
  } else {
    sessionCount += 1;
    thread_data **newSessions = (thread_data **)malloc(sizeof(thread_data *) * sessionCount);
    memcpy(newSessions, sessions, sizeof(thread_data *) * (sessionCount - 1));
    free(sessions);
    sessions = newSessions;
  }

  sessions[sessionCount - 1] = session;

  return sessionCount - 1;
}


#define TAG CLIENT_TAG("nfreerdp2")

static DWORD WINAPI tf_client_thread_proc(LPVOID arg)
{
	thread_data* data = (thread_data*)arg;
	DWORD nCount;
	DWORD status;
	HANDLE handles[64];
	
	freerdp* instance = data->instance;
	rdpChannels* channels = instance->context->channels;
	rdpContext*  context = instance->context;

	if (!freerdp_connect(instance))
	{
		WLog_ERR(TAG, "connection failure");
		return 0;
	}

	while (!freerdp_shall_disconnect(instance))
	{
		nCount = freerdp_get_event_handles(instance->context, &handles[0], 64);

		if (nCount == 0)
		{
			WLog_ERR(TAG, "%s: freerdp_get_event_handles failed", __FUNCTION__);
			break;
		}

		status = WaitForMultipleObjects(nCount, handles, FALSE, 100);

		if (status == WAIT_FAILED)
		{
			WLog_ERR(TAG, "%s: WaitForMultipleObjects failed with %"PRIu32"", __FUNCTION__,
				status);
			break;
		}
		

		if (!freerdp_check_event_handles(instance->context))
		{
			if (freerdp_get_last_error(instance->context) == FREERDP_ERROR_SUCCESS)
				WLog_ERR(TAG, "Failed to check FreeRDP event handles");

			break;
		}
		if(data->stopping){
			break;
		}
	}
	nodeContext *nc = (nodeContext*)instance->context;
	close_args *args = (close_args *)malloc(sizeof(close_args));
	generator_emit(nc->generatorContext, &CLOSE_GENERATOR_TYPE, args);
	
 	free(nc->generatorContext);
	freerdp_disconnect(instance);
	WSACleanup();
	freerdp_context_free(instance);
	freerdp_free(instance);

	ExitThread(0);
	return 0;
}

static BOOL tf_pre_connect(freerdp* instance)
{

	rdpSettings* settings;
	settings = instance->settings;
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
	settings->OrderSupport[NEG_MEMBLT_INDEX] = TRUE;
	settings->OrderSupport[NEG_MEM3BLT_INDEX] = TRUE;
	settings->OrderSupport[NEG_SAVEBITMAP_INDEX] = FALSE;
	settings->OrderSupport[NEG_GLYPH_INDEX_INDEX] = TRUE;
	settings->OrderSupport[NEG_FAST_INDEX_INDEX] = TRUE;
	settings->OrderSupport[NEG_FAST_GLYPH_INDEX] = TRUE;
	settings->OrderSupport[NEG_POLYGON_SC_INDEX] = FALSE;
	settings->OrderSupport[NEG_POLYGON_CB_INDEX] = FALSE;
	settings->OrderSupport[NEG_ELLIPSE_SC_INDEX] = FALSE;
	settings->OrderSupport[NEG_ELLIPSE_CB_INDEX] = FALSE;



	return TRUE;
}

BOOL tf_begin_paint(rdpContext* context)
{
	rdpGdi* gdi = context->gdi;
	gdi->primary->hdc->hwnd->invalid->null = TRUE;
	return TRUE;
}


struct draw_args {
  int x;
  int y;
  int w;
  int h;
  int bpp;
  BYTE* buffer;
};

Local<Array> draw_args_parser(void *generic) {
  draw_args *args = static_cast<draw_args *>(generic);

  Local<Object> obj = New<Object>();

  obj->Set(New<String>("x").ToLocalChecked(), New<Number>(args->x));
  obj->Set(New<String>("y").ToLocalChecked(), New<Number>(args->y));
  obj->Set(New<String>("w").ToLocalChecked(), New<Number>(args->w));
  obj->Set(New<String>("h").ToLocalChecked(), New<Number>(args->h));
  obj->Set(New<String>("bpp").ToLocalChecked(), New<Number>(args->bpp));

  int size = args->w * args->h * args->bpp;

  Nan::MaybeLocal<v8::Object> buffer = Nan::CopyBuffer((const char *)args->buffer, size);
  obj->Set(New<String>("buffer").ToLocalChecked(), buffer.ToLocalChecked());

  Local<Array> argv = New<Array>();
  argv->Set(0, obj);

  delete[] args->buffer;
  delete args;

  return argv;
}

const struct GeneratorType DRAW_GENERATOR_TYPE = {"bitmap",draw_args_parser};



static BOOL tf_end_paint(rdpContext* context)
{
	rdpGdi* gdi = context->gdi;
	if (gdi->primary->hdc->hwnd->invalid->null)
		return TRUE;

	int ninvalid = gdi->primary->hdc->hwnd->ninvalid;

	if (ninvalid < 1)
		return TRUE;

	draw_args *args = new draw_args;
	args->x = gdi->primary->hdc->hwnd->invalid->x;
	args->y = gdi->primary->hdc->hwnd->invalid->y;
	args->w = gdi->primary->hdc->hwnd->invalid->w;
	args->h = gdi->primary->hdc->hwnd->invalid->h;


	args->bpp = 4;

	int size = args->w * args->h * args->bpp;
	args->buffer = new BYTE[size];

	int dest_pos = 0;
	int dest_line_width = args->w * args->bpp;
	for(int i = args->y; i < args->y + args->h; i++) {
	// memcopy only columns that are relevant
		int start_pos = (i * gdi->width * args->bpp) + (args->x * args->bpp);
		BYTE* src = &gdi->primary_buffer[start_pos];
		BYTE* dest = &args->buffer[dest_pos];
		memcpy(dest, src, dest_line_width);
		dest_pos += dest_line_width;
	}

	nodeContext *nc = (nodeContext*)context;
	generator_emit(nc->generatorContext, &DRAW_GENERATOR_TYPE, args);

	//tf_save_dib(context, args->w, args->h, context->gdi->dstFormat, args->buffer, NULL);

	return TRUE;
}

static BOOL tf_post_connect(freerdp* instance)
{
	if (!gdi_init(instance, PIXEL_FORMAT_RGBX32))
		return FALSE;
		
	instance->update->BeginPaint = tf_begin_paint;
	instance->update->EndPaint = tf_end_paint;
	
	nodeContext *nc = (nodeContext*)instance->context;
	connect_args *args = (connect_args *)malloc(sizeof(connect_args));
	generator_emit(nc->generatorContext, &CONNECT_GENERATOR_TYPE, args);
	return TRUE;
}

static BOOL tf_context_new(freerdp* instance, rdpContext* context)
{
	return TRUE;
}

static void tf_context_free(freerdp* instance, rdpContext* context)
{
}


int node_freerdp_connect(int argc, char* argv[], Callback *callback)
{
	WSADATA wsaData;
	WSAStartup(0x101, &wsaData);
  int status;
	HANDLE thread;
	freerdp* instance;
	instance = freerdp_new();
 	nodeContext* nContext;
	struct thread_data* data;

	if (!instance)
	{
		WLog_ERR(TAG, "Couldn't create instance");
		return 1;
	}

	instance->PreConnect = tf_pre_connect;
	instance->PostConnect = tf_post_connect;
	instance->ContextSize = sizeof(nodeContext);
	instance->ContextNew = tf_context_new;
	instance->ContextFree = tf_context_free;
 	freerdp_context_new(instance);

	nContext = (nodeContext*)instance->context;
	nContext->generatorContext = new GeneratorContext;
	nContext->generatorContext->callback = callback;



	freerdp_register_addin_provider(freerdp_channels_load_static_addin_entry, 0);

	// if (!freerdp_context_new(instance))
	// {
	// 	WLog_ERR(TAG, "Couldn't create context");
	// 	return 1;
	// }

	status = freerdp_client_settings_parse_command_line(instance->settings, argc,
		argv, FALSE);

	if (status < 0)
	{
		return 0;
	}

	if (!freerdp_client_load_addins(instance->context->channels,
		instance->settings))
		return -1;

	data = (struct thread_data*) malloc(sizeof(struct thread_data));
	ZeroMemory(data, sizeof(sizeof(struct thread_data)));
	data->instance = instance;
	data->stopping = false;


	thread = CreateThread(NULL, 0, tf_client_thread_proc, data, 0, NULL);
	
	
	int index = add_session(data);

	// WSACleanup();
	// freerdp_context_free(instance);
	// freerdp_free(instance);

    return index;
}


void node_freerdp_close(int session_index)
{
  // NOTE: Doesn't block on closed session, will send closed event when completed
  thread_data *session = sessions[session_index];
  session->stopping = true;
  //freerdp_disconnect(session->instance);
}


void node_freerdp_send_key_event_scancode(int session_index, int code, int pressed)
{
  thread_data* session = sessions[session_index];
  freerdp* instance = session->instance;
  rdpInput* input = instance->input;

  freerdp_input_send_keyboard_event_ex(input, pressed, code);
}
void node_freerdp_send_pointer_event(int session_index, int flags, int x, int y)
{
  thread_data* session = sessions[session_index];
  freerdp* instance = session->instance;
  rdpInput* input = instance->input;

  freerdp_input_send_mouse_event(input, flags, x, y);
}
