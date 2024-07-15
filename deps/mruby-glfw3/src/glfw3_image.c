#include <stdbool.h>
#include <string.h>

#include <mruby.h>
#include <mruby/data.h>
#include <mruby/array.h>
#include <mruby/numeric.h>

#include <GLFW/glfw3.h>
#include "glfw3_image.h"
#include "glfw3_private.h"

#define NUM_OF_CHANNELS 4

typedef struct pixel_
{
  union {
    unsigned int val;
    struct {
      unsigned char r, g, b, a;
    };
  };
} pixel_t;

static struct RClass *mrb_glfw3_image_class;

void
mrb_glfw3_image_free(mrb_state *mrb, void *ptr)
{
  GLFWimage *img = ptr;
  if (img) {
    if (img->pixels) {
      mrb_free(mrb, img->pixels);
      img->pixels = NULL;
    }
    mrb_free(mrb, img);
  }
}

const struct mrb_data_type mrb_glfw3_image_type = { "GLFWimage", mrb_glfw3_image_free };

static inline GLFWimage*
get_image(mrb_state *mrb, mrb_value self)
{
  return (GLFWimage*)mrb_data_get_ptr(mrb, self, &mrb_glfw3_image_type);
}

static inline int
pixel_size()
{
  return sizeof(char) * NUM_OF_CHANNELS;
}

static inline int
calc_image_size(GLFWimage *image)
{
  return image->width * image->height;
}

static inline int
calc_image_pixels_size(GLFWimage *image)
{
  return calc_image_size(image) * pixel_size();
}

static inline pixel_t*
image_pixels(GLFWimage *image)
{
  return (pixel_t*)(&image->pixels[0]);
}

static mrb_value
image_initialize(mrb_state *mrb, mrb_value self)
{
  GLFWimage *image;
  mrb_int w;
  mrb_int h;
  int size;
  mrb_get_args(mrb, "ii", &w, &h);
  if (0 > w || 0 > h) {
    mrb_raise(mrb, E_ARGUMENT_ERROR, "Image dimensions are invalid!");
  }
  image = mrb_malloc(mrb, sizeof(GLFWimage));
  image->width = w;
  image->height = h;
  size = calc_image_pixels_size(image);
  image->pixels = mrb_malloc(mrb, size);
  memset(&image->pixels[0], 0, size);
  DATA_PTR(self) = image;
  DATA_TYPE(self) = &mrb_glfw3_image_type;
  return self;
}

static mrb_value
image_get_memsize(mrb_state *mrb, mrb_value self)
{
  return mrb_fixnum_value(calc_image_pixels_size(get_image(mrb, self)));
}

static mrb_value
image_get_pixelsize(mrb_state *mrb, mrb_value self)
{
  return mrb_fixnum_value(sizeof(pixel_t));
}

static mrb_value
image_get_width(mrb_state *mrb, mrb_value self)
{
  return mrb_fixnum_value(get_image(mrb, self)->width);
}

static mrb_value
image_get_height(mrb_state *mrb, mrb_value self)
{
  return mrb_fixnum_value(get_image(mrb, self)->height);
}

static mrb_value
image_clear(mrb_state *mrb, mrb_value self)
{
  GLFWimage *image;
  mrb_value a;
  pixel_t pixel;
  mrb_get_args(mrb, "A", &a);
  pixel.r = mrb_int(mrb, mrb_ary_ref(mrb, a, 0)) & 0xFF;
  pixel.g = mrb_int(mrb, mrb_ary_ref(mrb, a, 1)) & 0xFF;
  pixel.b = mrb_int(mrb, mrb_ary_ref(mrb, a, 2)) & 0xFF;
  pixel.a = mrb_int(mrb, mrb_ary_ref(mrb, a, 3)) & 0xFF;
  image = get_image(mrb, self);
  memset(image->pixels, pixel.val, calc_image_pixels_size(image));
  return self;
}

static mrb_value
image_aget(mrb_state *mrb, mrb_value self)
{
  GLFWimage *image;
  mrb_int x;
  mrb_int y;
  pixel_t pixel;
  mrb_value vals[4];
  mrb_get_args(mrb, "ii", &x, &y);
  image = get_image(mrb, self);
  if (x < 0 || image->width < x || y < 0 || image->height < y) {
    return mrb_glfw3_ary_of(mrb, 4, mrb_fixnum_value(0));
  }
  pixel = image_pixels(image)[x + y * image->width];
  vals[0] = mrb_fixnum_value(pixel.r);
  vals[1] = mrb_fixnum_value(pixel.g);
  vals[2] = mrb_fixnum_value(pixel.b);
  vals[3] = mrb_fixnum_value(pixel.a);
  return mrb_ary_new_from_values(mrb, 4, vals);
}

static mrb_value
image_aset(mrb_state *mrb, mrb_value self)
{
  GLFWimage *image;
  mrb_int x;
  mrb_int y;
  pixel_t *pixels;
  mrb_value val;
  mrb_get_args(mrb, "iiA", &x, &y, &val);
  image = get_image(mrb, self);
  if (x < 0 || image->width < x || y < 0 || image->height < y) {
    return mrb_nil_value();
  }
  pixels = &image_pixels(image)[x + y * image->width];
  pixels->r = mrb_int(mrb, mrb_ary_ref(mrb, val, 0)) & 0xFF;
  pixels->g = mrb_int(mrb, mrb_ary_ref(mrb, val, 1)) & 0xFF;
  pixels->b = mrb_int(mrb, mrb_ary_ref(mrb, val, 2)) & 0xFF;
  pixels->a = mrb_int(mrb, mrb_ary_ref(mrb, val, 3)) & 0xFF;
  return mrb_nil_value();
}

void
mrb_glfw3_image_init(mrb_state *mrb, struct RClass *mod)
{
  mrb_glfw3_image_class = mrb_define_class_under(mrb, mod, "Image", mrb->object_class);
  MRB_SET_INSTANCE_TT(mrb_glfw3_image_class, MRB_TT_DATA);
  mrb_define_method(mrb, mrb_glfw3_image_class, "initialize", image_initialize,    MRB_ARGS_REQ(2));
  mrb_define_method(mrb, mrb_glfw3_image_class, "memsize",    image_get_memsize,   MRB_ARGS_NONE());
  mrb_define_method(mrb, mrb_glfw3_image_class, "pixelsize",  image_get_pixelsize, MRB_ARGS_NONE());
  mrb_define_method(mrb, mrb_glfw3_image_class, "width",      image_get_width,     MRB_ARGS_NONE());
  mrb_define_method(mrb, mrb_glfw3_image_class, "height",     image_get_height,    MRB_ARGS_NONE());
  mrb_define_method(mrb, mrb_glfw3_image_class, "clear",      image_clear,         MRB_ARGS_REQ(1));
  mrb_define_method(mrb, mrb_glfw3_image_class, "[]",         image_aget,          MRB_ARGS_REQ(2));
  mrb_define_method(mrb, mrb_glfw3_image_class, "[]=",        image_aset,          MRB_ARGS_REQ(3));
}
