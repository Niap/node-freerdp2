#include "context.h"
#include "rail.h"
#include "clipboard.h"

#include <freerdp/gdi/gfx.h>
#include <freerdp/log.h>

void node_OnChannelConnectedEventHandler(void* context,
	ChannelConnectedEventArgs* e)
{
	nodeContext* nctx = (nodeContext*)context;
	rdpSettings* settings = nctx->context.settings;
	if (strcmp(e->name, RAIL_SVC_CHANNEL_NAME) == 0)
	{
		node_rail_init(nctx, (RailClientContext*)e->pInterface);
	}else if (strcmp(e->name, CLIPRDR_SVC_CHANNEL_NAME) == 0)
	{
		node_cliprdr_init(nctx, (CliprdrClientContext*)e->pInterface);
	}
}

void node_OnChannelDisconnectedEventHandler(void* context,
	ChannelDisconnectedEventArgs* e)
{
	nodeContext* nctx = (nodeContext*)context;
	rdpSettings* settings = nctx->context.settings;

	if (strcmp(e->name, RAIL_SVC_CHANNEL_NAME) == 0)
	{
		node_rail_uninit(nctx, (RailClientContext*)e->pInterface);
	}else if (strcmp(e->name, CLIPRDR_SVC_CHANNEL_NAME) == 0)
	{
		node_cliprdr_uninit(nctx, (CliprdrClientContext*)e->pInterface);
	}
	
}
