#ifndef NODE_ASYNC_LEARN_GENERATOR_H
#define NODE_ASYNC_LEARN_GENERATOR_H

#include <nan.h>

using Nan::Callback;
using v8::Local;
using v8::Value;
using v8::Array;

struct GeneratorContext {
  Callback *callback;  // javascript callback
};

struct GeneratorType {
  const char *name;
  Local<Array> (*arg_parser)(void *);
};

void generator_emit(GeneratorContext *generator, const GeneratorType *type, void *data);

struct connect_args {};
Local<Array> connect_args_parser(void *generic);
const struct GeneratorType CONNECT_GENERATOR_TYPE = {"connect",connect_args_parser};

struct close_args {
	char * msg;
};
Local<Array> close_args_parser(void *generic);
const struct GeneratorType CLOSE_GENERATOR_TYPE = {"close",close_args_parser };

struct draw_args {
  int x;
  int y;
  int w;
  int h;
  int bpp;
  BYTE* buffer;
};
Local<Array> draw_args_parser(void *generic) ;
const struct GeneratorType DRAW_GENERATOR_TYPE = {"bitmap",draw_args_parser};


#endif // NODE_ASYNC_LEARN_GENERATOR_H
