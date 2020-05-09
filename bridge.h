#ifndef NODE_FREERDP_BRIDGE_H
#define NODE_FREERDP_BRIDGE_H

#include <nan.h>

NAN_METHOD(Connect);
NAN_METHOD(Close);
NAN_METHOD(SendKeyEventScancode);
NAN_METHOD(SendPointerEvent);
NAN_METHOD(SendClipboard);

#endif  // NODE_FREERDP_BRIDGE_H