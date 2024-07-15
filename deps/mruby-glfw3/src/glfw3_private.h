#ifndef MRB_GLFW3_PRIVATE_H
#define MRB_GLFW3_PRIVATE_H

#include <string.h>

#include <mruby.h>
#include <mruby/data.h>
#include <mruby/variable.h>
#include <mruby/string.h>
#include <mruby/array.h>
#include <mruby/class.h>

#define E_GLFW_ERROR (mrb_class_get(mrb, "GLFWError"))

static inline mrb_value
mrb_glfw3_ary_delete(mrb_state *mrb, mrb_value ary, mrb_value obj)
{
  return mrb_funcall(mrb, ary, "delete", 1, obj);
}

static inline mrb_value
mrb_glfw3_cache(mrb_state *mrb)
{
  return mrb_iv_get(mrb,
                    mrb_obj_value(mrb_module_get(mrb, "GLFW")),
                    mrb_intern_lit(mrb, "__glfw_objects"));
}

static inline void
mrb_glfw3_cache_object(mrb_state *mrb, mrb_value obj)
{
  mrb_ary_push(mrb, mrb_glfw3_cache(mrb), obj);
}

static inline void
mrb_glfw3_uncache_object(mrb_state *mrb, mrb_value obj)
{
  mrb_glfw3_ary_delete(mrb, mrb_glfw3_cache(mrb), obj);
}

static inline int
mrb_glfw3_unpack_str_as_int(mrb_state *mrb, char *str)
{
  if (strlen(str) != 4) {
    mrb_raise(mrb, E_ARGUMENT_ERROR, "String must be length of 4 to unpack as int");
  }
  return *((int*)str);
}

static inline mrb_value
mrb_glfw3_ary_of(mrb_state *mrb, mrb_int size, mrb_value val)
{
  int i;
  mrb_value ary = mrb_ary_new(mrb);
  for (i = 0; i < size; ++i) {
    mrb_ary_push(mrb, ary, val);
  }
  return ary;
}

#endif
