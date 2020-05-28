#include "pointer.h"

struct pointer_args {
  int w;
  int h;
  BYTE* buffer;
};

Local<Array> pointer_args_parser(void *generic) {
  pointer_args *args = static_cast<pointer_args *>(generic);

  Local<Object> obj = New<Object>();

  obj->Set(New<String>("w").ToLocalChecked(), New<Number>(args->w));
  obj->Set(New<String>("h").ToLocalChecked(), New<Number>(args->h));

  int size = args->w * args->h *4;

  Nan::MaybeLocal<v8::Object> buffer = Nan::CopyBuffer((const char *)args->buffer, size);
  obj->Set(New<String>("buffer").ToLocalChecked(), buffer.ToLocalChecked());

  Local<Array> argv = New<Array>();
  argv->Set(0, obj);

  delete[] args->buffer;
  delete args;

  return argv;
}

const struct GeneratorType POINTER_GENERATOR_TYPE = {"pointer",pointer_args_parser};

BOOL node_Pointer_New(rdpContext* context, const rdpPointer* pointer)
{
	int size = pointer->height * pointer->width * GetBytesPerPixel(PIXEL_FORMAT_RGBA32);
	BYTE* pdata = (BYTE*)_aligned_malloc( size, 16 );
	rdpGdi* gdi = context->gdi;

	freerdp_image_copy_from_pointer_data(pdata, PIXEL_FORMAT_RGBA32, 0, 0, 0,
		pointer->width, pointer->height,
		pointer->xorMaskData, pointer->lengthXorMask,
		pointer->andMaskData, pointer->lengthAndMask, pointer->xorBpp, &gdi->palette);

    ((NodePointer *)pointer)->buffer = pdata;

	return TRUE;
}

BOOL node_Pointer_Free(rdpContext* context, rdpPointer* pointer)
{
	byte * hCur;

	if (!pointer)
		return FALSE;

	hCur = ((NodePointer*)pointer)->buffer;

	if (hCur != 0)
		_aligned_free(hCur);

	return TRUE;
}

BOOL node_Pointer_Set(rdpContext* context, const rdpPointer* pointer)
{
	if (!pointer)
		return FALSE;
	nodeContext *nc = (nodeContext*)context;
	byte * buffer = ((NodePointer*)pointer)->buffer;
    pointer_args *args = new pointer_args;
    args->w = pointer->width;
    args->h = pointer->height;
    int size = args->w * args->h *4;
    args->buffer = new BYTE[size];
	memcpy(args->buffer, buffer, size);
    
	generator_emit(nc->generatorContext, &POINTER_GENERATOR_TYPE, args);

	return TRUE;
}

BOOL node_register_pointer(rdpGraphics* graphics)
{
	rdpPointer pointer;

	if (!graphics)
		return FALSE;

	ZeroMemory(&pointer, sizeof(rdpPointer));
	pointer.size = sizeof(NodePointer);
	pointer.New = reinterpret_cast<pPointer_New>(node_Pointer_New);
	pointer.Free = reinterpret_cast<pPointer_Free>(node_Pointer_Free);
	pointer.Set = reinterpret_cast<pPointer_Set>(node_Pointer_Set);
	graphics_register_pointer(graphics, &pointer);
	return TRUE;
}
