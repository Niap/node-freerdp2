#ifndef NODE_FREERDP_CONTEXT_H
#define NODE_FREERDP_CONTEXT_H

#include <freerdp/freerdp.h>

#include <freerdp/client/rail.h>
#include <freerdp/client/cliprdr.h>

#include "generator.h"

typedef struct _NodeClipboard {
	byte* buffer;
	int length;
} NodeClipboard;


struct node_info
{
  void* data;
};
typedef struct node_info nodeInfo;

struct node_context
{
  rdpContext context;
	DEFINE_RDP_CLIENT_COMMON();

  nodeInfo* nodei;
  GeneratorContext *generatorContext;

  //ANDROID_EVENT_QUEUE* event_queue;
  //pthread_t thread;
  //BOOL is_connected;

	RailClientContext* rail;
  NodeClipboard * clipboard;
  CliprdrClientContext* clipboard_context;
};
typedef struct node_context nodeContext;

#endif // NODE_FREERDP_CONTEXT_H
