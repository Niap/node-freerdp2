#ifndef NODE_FREERDP_BRIDGE_H
#define NODE_FREERDP_BRIDGE_H

#include <nan.h>

NAN_METHOD(Connect);
NAN_METHOD(Close);
NAN_METHOD(SendKeyEventScancode);
NAN_METHOD(SendPointerEvent);

#endif  // NODE_FREERDP_BRIDGE_H