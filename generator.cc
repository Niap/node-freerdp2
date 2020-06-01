#include <nan.h>

#include "generator.h"

using Nan::Callback;
using Nan::HandleScope;
using Nan::New;
using Nan::Null;
using v8::Array;
using v8::Local;
using v8::String;
using v8::Value;
using v8::Number;
using v8::Object;

struct GeneratorBaton {
  uv_work_t request;     // libuv

  Callback *callback;    // javascript callback

  const GeneratorType *type;
  void *data;
};

// called by libuv worker in separate thread
static void DelayAsync(uv_work_t *req) { }

static void DelayAsyncAfter(uv_work_t *req, int status) {
  HandleScope scope;

  GeneratorBaton *baton = static_cast<GeneratorBaton *>(req->data);

  Local<Array> env_args = baton->type->arg_parser(baton->data);

  Local<Value> argv[] = {
    New<String>(baton->type->name).ToLocalChecked(),
    env_args
  };

  baton->callback->Call(2, argv);

  delete baton;
}

void generator_emit(GeneratorContext *generator, const GeneratorType *type, void * data) {
  // Create context for work queue
  GeneratorBaton *baton = new GeneratorBaton;
  baton->callback = generator->callback;
  baton->type = type;
  baton->data = data;

  baton->request.data = baton;

  uv_queue_work(uv_default_loop(), &baton->request, DelayAsync, DelayAsyncAfter);
}

Local<Array> connect_args_parser(void *generic) {
  connect_args *args = static_cast<connect_args *>(generic);
  Local<Array> argv = New<Array>();
  free(args);
  return argv;
}
Local<Array> close_args_parser(void *generic) {
  close_args *args = static_cast<close_args *>(generic);
  Local<Array> argv = New<Array>();
  Local<Object> obj = New<Object>();
  obj->Set(New<String>("msg").ToLocalChecked(), New<String>(args->msg).ToLocalChecked());
  argv->Set(0, obj);
  free(args);
  return argv;
}

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
