#ifndef NODE_FREERDP_RDP_H
#define NODE_FREERDP_RDP_H

#include "context.h"

#define TAG CLIENT_TAG("NodeFreerdp")


int node_freerdp_connect(int argc, char* argv[], Callback *callback);
void node_freerdp_send_key_event_scancode(int session_index, int code, int pressed);
void node_freerdp_send_pointer_event(int session_index, int flags, int x, int y);
void node_freerdp_close(int session);
void node_freerdp_cliprdr_set_data(int session_index,byte* clipboardData,int length) ;
void node_freerdp_request_keyframe(int session_index) ;


#endif // NODE_FREERDP_RDP_H