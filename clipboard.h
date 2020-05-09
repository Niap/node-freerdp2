#include "context.h"

BOOL node_cliprdr_init(nodeContext* swfc, CliprdrClientContext* cliprdr);
void node_cliprdr_uninit(nodeContext* swfc, CliprdrClientContext* cliprdr);

UINT cliprdr_send_format_list(CliprdrClientContext* cliprdr) ;