#include "clipboard.h"


UINT cliprdr_send_capabilities(CliprdrClientContext* clipboard) {

	CLIPRDR_GENERAL_CAPABILITY_SET cap_set;
	cap_set.capabilitySetType = CB_CAPSTYPE_GENERAL; 
	cap_set.capabilitySetLength = 12;
	cap_set.version = CB_CAPS_VERSION_2;
	cap_set.generalFlags = CB_USE_LONG_FORMAT_NAMES;

	CLIPRDR_CAPABILITIES caps;
	caps.cCapabilitiesSets = 1;
	caps.capabilitySets = (CLIPRDR_CAPABILITY_SET*)&cap_set;

	return clipboard->ClientCapabilities(clipboard, &caps);
}

UINT cliprdr_send_format_list(CliprdrClientContext* cliprdr) {
	CLIPRDR_FORMAT* formats = (CLIPRDR_FORMAT*) calloc(1, sizeof(CLIPRDR_FORMAT));
	CLIPRDR_FORMAT_LIST format_list;

	formats[0].formatId = CF_TEXT;

	format_list.formats = formats;
	format_list.numFormats = 1;

	return cliprdr->ClientFormatList(cliprdr, &format_list);
}


UINT node_cliprdr_monitor_ready(CliprdrClientContext* context,
	CLIPRDR_MONITOR_READY* monitorReady){
	UINT rc = ERROR_INTERNAL_ERROR;
	if (!context || !monitorReady)
		return ERROR_INTERNAL_ERROR;
	rc = cliprdr_send_capabilities(context);
	if (rc != CHANNEL_RC_OK)
		return rc;
	return cliprdr_send_format_list(context);
}

UINT node_cliprdr_server_format_data_request(CliprdrClientContext* context, CLIPRDR_FORMAT_DATA_REQUEST* formatDataRequest) {
	UINT rc = CHANNEL_RC_OK;
	CLIPRDR_FORMAT_DATA_RESPONSE response;

	nodeContext * nctx = (nodeContext *)context->custom;
	if( nctx->clipboard->length >0 ){
		response.msgFlags = CB_RESPONSE_OK;
		response.dataLen = nctx->clipboard->length;
		response.requestedFormatData = nctx->clipboard->buffer;
		rc = context->ClientFormatDataResponse(context,&response);
	}
	
	
	return rc;
}

UINT node_cliprdr_server_format_data_response(CliprdrClientContext* context,CLIPRDR_FORMAT_DATA_RESPONSE* format_data_response) {
	return CHANNEL_RC_OK;
}



BOOL node_cliprdr_init(nodeContext* swfc, CliprdrClientContext* cliprdr) {

	NodeClipboard * clipboard = (NodeClipboard*)malloc(sizeof(NodeClipboard));
	char * defaultClip = "who is your daddy.";
	clipboard->length = strlen(defaultClip)+1;
	clipboard->buffer = (byte *)defaultClip;
	swfc->clipboard = clipboard;
	swfc->clipboard_context = cliprdr;
	cliprdr->custom = swfc;

	cliprdr->MonitorReady = node_cliprdr_monitor_ready;
	cliprdr->ServerFormatDataRequest = node_cliprdr_server_format_data_request;
	cliprdr->ServerFormatDataResponse = node_cliprdr_server_format_data_response;

	return TRUE;
}


void node_cliprdr_uninit(nodeContext* swfc, CliprdrClientContext* cliprdr) {
	swfc->clipboard = NULL;
	cliprdr->custom = NULL;
}