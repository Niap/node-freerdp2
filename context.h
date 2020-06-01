#ifndef NODE_FREERDP_CONTEXT_H
#define NODE_FREERDP_CONTEXT_H

#pragma comment(lib, "DbgHelp.lib")

#include <winsock2.h>
#include <Windows.h>
#include <ws2tcpip.h>
#include <stdio.h>

#include <freerdp/freerdp.h>
#include <freerdp/constants.h>
#include <freerdp/log.h>
#include <freerdp/gdi/gdi.h>
#include <freerdp/client/cmdline.h>
#include <freerdp/client/channels.h>
#include <freerdp/channels/channels.h>
#include <freerdp/client/rail.h>
#include <freerdp/client/cliprdr.h>

#include <winpr/crt.h>
#include <winpr/synch.h>


#include "generator.h"

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


typedef struct _SessionData
{
	freerdp* instance;
	bool stopping;
}SessionData;
typedef struct _NodePointer
{
	rdpPointer pointer;
	byte * buffer;
  int width;
  int height;
}NodePointer;

typedef struct _NodeClipboard {
	byte* buffer;
	int length;
} NodeClipboard;

struct node_context
{
  rdpContext context;
  DEFINE_RDP_CLIENT_COMMON();
  SessionData * session;
  GeneratorContext *generatorContext;
	RailClientContext* rail;
  NodeClipboard * clipboard;
  CliprdrClientContext* clipboardContext;
  bool keyframe;
};
typedef struct node_context nodeContext;

#endif // NODE_FREERDP_CONTEXT_H
