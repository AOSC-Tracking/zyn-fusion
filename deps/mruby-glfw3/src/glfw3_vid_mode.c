#include <stdbool.h>
#include <string.h>

#include <mruby.h>
#include <mruby/array.h>
#include <mruby/class.h>
#include <mruby/data.h>
#include <mruby/hash.h>
#include <mruby/variable.h>
#include <mruby/string.h>

#include <GLFW/glfw3.h>

#include "glfw3_vid_mode.h"

static struct RClass *mrb_glfw3_vid_mode_class;

void
mrb_glfw3_vid_mode_free(mrb_state *mrb, void *ptr)
{
  if (ptr) {
    mrb_free(mrb, ptr);
  }
}

const struct mrb_data_type mrb_glfw3_vid_mode_type = { "GLFWvidmode", mrb_glfw3_vid_mode_free };

mrb_value
mrb_glfw3_vid_mode_value(mrb_state *mrb, GLFWvidmode vidmode)
{
  GLFWvidmode *vmode;
  mrb_value result = mrb_obj_new(mrb, mrb_glfw3_vid_mode_class, 0, NULL);
  vmode = mrb_malloc(mrb, sizeof(GLFWvidmode));
  *vmode = vidmode;
  DATA_PTR(result) = vmode;
  DATA_TYPE(result) = &mrb_glfw3_vid_mode_type;
  return result;
}

static GLFWvidmode*
get_vid_mode(mrb_state *mrb, mrb_value self)
{
  return (GLFWvidmode*)mrb_data_get_ptr(mrb, self, &mrb_glfw3_vid_mode_type);
}

static mrb_value
vid_mode_width(mrb_state *mrb, mrb_value self)
{
  return mrb_fixnum_value(get_vid_mode(mrb, self)->width);
}

static mrb_value
vid_mode_height(mrb_state *mrb, mrb_value self)
{
  return mrb_fixnum_value(get_vid_mode(mrb, self)->height);
}

static mrb_value
vid_mode_red_bits(mrb_state *mrb, mrb_value self)
{
  return mrb_fixnum_value(get_vid_mode(mrb, self)->redBits);
}

static mrb_value
vid_mode_green_bits(mrb_state *mrb, mrb_value self)
{
  return mrb_fixnum_value(get_vid_mode(mrb, self)->greenBits);
}

static mrb_value
vid_mode_blue_bits(mrb_state *mrb, mrb_value self)
{
  return mrb_fixnum_value(get_vid_mode(mrb, self)->blueBits);
}

static mrb_value
vid_mode_refresh_rate(mrb_state *mrb, mrb_value self)
{
  return mrb_fixnum_value(get_vid_mode(mrb, self)->refreshRate);
}

void
mrb_glfw3_vid_mode_init(mrb_state *mrb, struct RClass *mod)
{
  mrb_glfw3_vid_mode_class = mrb_define_class_under(mrb, mod, "VidMode", mrb->object_class);
  MRB_SET_INSTANCE_TT(mrb_glfw3_vid_mode_class, MRB_TT_DATA);
  mrb_define_method(mrb, mrb_glfw3_vid_mode_class, "width",        vid_mode_width,        MRB_ARGS_NONE());
  mrb_define_method(mrb, mrb_glfw3_vid_mode_class, "height",       vid_mode_height,       MRB_ARGS_NONE());
  mrb_define_method(mrb, mrb_glfw3_vid_mode_class, "red_bits",     vid_mode_red_bits,     MRB_ARGS_NONE());
  mrb_define_method(mrb, mrb_glfw3_vid_mode_class, "green_bits",   vid_mode_green_bits,   MRB_ARGS_NONE());
  mrb_define_method(mrb, mrb_glfw3_vid_mode_class, "blue_bits",    vid_mode_blue_bits,    MRB_ARGS_NONE());
  mrb_define_method(mrb, mrb_glfw3_vid_mode_class, "refresh_rate", vid_mode_refresh_rate, MRB_ARGS_NONE());
}
