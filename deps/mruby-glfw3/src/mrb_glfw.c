#include <stdbool.h>

#include <mruby.h>
#include <mruby/class.h>
#include <mruby/variable.h>
#include <mruby/array.h>
#include <mruby/error.h>

#include "glfw3_private.h"
#include "glfw3_cursor.h"
#include "glfw3_gamma_ramp.h"
#include "glfw3_image.h"
#include "glfw3_monitor.h"
#include "glfw3_vid_mode.h"
#include "glfw3_window.h"

/* Needed for error callbacks to work correctly */
static mrb_state *err_M = NULL;

static mrb_value
glfw_init(mrb_state *mrb, mrb_value self)
{
  int err;
  err = glfwInit();
  if (err != GL_TRUE) {
    mrb_raise(mrb, E_GLFW_ERROR, "GLFW initialization failed.");
  }
  return mrb_bool_value(true);
}

static void
glfw_terminate_m(mrb_state *mrb)
{
  mrb_value objs = mrb_glfw3_cache(mrb);
  for (mrb_int i = 0; i < RARRAY_LEN(objs); ++i) {
    mrb_value const obj = RARRAY_PTR(objs)[i];
    if (DATA_TYPE(obj) == &mrb_glfw3_window_type) {
      if (DATA_PTR(obj)) {
        glfwDestroyWindow((GLFWwindow*)DATA_PTR(obj));
        DATA_PTR(obj) = NULL;
        DATA_TYPE(obj) = NULL;
      }
    } else {
      mrb_assert(false);
    }
  }
  glfwTerminate();
}

static mrb_value
glfw_terminate(mrb_state *mrb, mrb_value klass)
{
  glfw_terminate_m(mrb);
  mrb_ary_clear(mrb, mrb_glfw3_cache(mrb));
  return mrb_nil_value();
}

static void
glfw_error_func(int code, char const* str)
{
  if (err_M) {
    mrb_raisef(err_M, mrb_class_get(err_M, "GLFWError"), "%S: %S",
               mrb_fixnum_value(code), mrb_str_new_cstr(err_M, str));
  } else {
    fprintf(stderr, "GLFW error: %s (%d)\n", str, code);
  }
}

static mrb_value
glfw_current_context(mrb_state *M, mrb_value self)
{
  GLFWwindow * const cur = glfwGetCurrentContext();
  if (cur) {
    return mrb_obj_value(glfwGetWindowUserPointer(cur));
  } else {
    return mrb_nil_value();
  }
}

static mrb_value
glfw_set_swap_interval(mrb_state *M, mrb_value self)
{
  mrb_int v;
  mrb_get_args(M, "i", &v);
  return glfwSwapInterval(v), mrb_fixnum_value(v);
}

static mrb_value
glfw_proc_address(mrb_state *mrb, mrb_value self)
{
  /* TODO: figure out how to static assert nicely without C++ */
  /*static_assert(sizeof(void*) == sizeof(GLFWglproc), "function pointer and pointer size must be same");*/
  char *str;
  mrb_get_args(mrb, "z", &str);
  return mrb_cptr_value(mrb, (void*)glfwGetProcAddress(str));
}

static mrb_value
glfw_extension_supported_p(mrb_state *M, mrb_value self)
{
  char *str;
  mrb_get_args(M, "z", &str);
  return mrb_bool_value(glfwExtensionSupported(str) != GL_FALSE);
}

static mrb_value
glfw_version(mrb_state *M, mrb_value self)
{
  int major, minor, rev;
  mrb_value vers[3];
  glfwGetVersion(&major, &minor, &rev);
  vers[0] = mrb_fixnum_value(major);
  vers[1] = mrb_fixnum_value(minor);
  vers[2] = mrb_fixnum_value(rev);
  return mrb_ary_new_from_values(M, 3, vers);
}

static mrb_value
glfw_version_string(mrb_state *M, mrb_value self)
{
  return mrb_str_new_cstr(M, glfwGetVersionString());
}

static mrb_value
glfw_get_time(mrb_state *M, mrb_value self)
{
  return mrb_float_value(M, glfwGetTime());
}

static mrb_value
glfw_set_time(mrb_state *M, mrb_value self)
{
  mrb_float v;
  mrb_get_args(M, "f", &v);
  glfwSetTime(v);
  return mrb_float_value(M, v);
}

static mrb_value
glfw_poll_events(mrb_state *M, mrb_value self)
{
  glfwPollEvents();
  return self;
}

static mrb_value
glfw_wait_events(mrb_state *M, mrb_value self)
{
  //glfwWaitEvents();
  return self;
}

static mrb_value
glfw_default_window_hints(mrb_state *M, mrb_value self)
{
  glfwDefaultWindowHints();
  return self;
}

static mrb_value
glfw_window_hint(mrb_state *mrb, mrb_value self)
{
  mrb_int target, hint;
  mrb_get_args(mrb, "ii", &target, &hint);
  glfwWindowHint(target, hint);
  return self;
}

static mrb_value
glfw_post_empty_event(mrb_state *mrb, mrb_value self)
{
  glfwPostEmptyEvent();
  return self;
}

static mrb_value
glfw_joystick_present(mrb_state *mrb, mrb_value self)
{
  mrb_int joy;
  mrb_get_args(mrb, "i", &joy);
  return mrb_fixnum_value(glfwJoystickPresent(joy));
}

static mrb_value
glfw_joystick_axes(mrb_state *mrb, mrb_value self)
{
  mrb_int joy;
  int count;
  int i;
  const float *axes;
  mrb_value result;
  mrb_get_args(mrb, "i", &joy);
  axes = glfwGetJoystickAxes(joy, &count);
  result = mrb_ary_new(mrb);
  for (i = 0; i < count; ++i) {
    mrb_ary_push(mrb, result, mrb_float_value(mrb, axes[i]));
  }
  return result;
}

static mrb_value
glfw_joystick_buttons(mrb_state *mrb, mrb_value self)
{
  mrb_int joy;
  int count;
  int i;
  const unsigned char *buttons;
  mrb_value result;
  mrb_get_args(mrb, "i", &joy);
  buttons = glfwGetJoystickButtons(joy, &count);
  result = mrb_ary_new(mrb);
  for (i = 0; i < count; ++i) {
    mrb_ary_push(mrb, result, mrb_fixnum_value(buttons[i]));
  }
  return result;
}

static mrb_value
glfw_joystick_name(mrb_state *mrb, mrb_value self)
{
  mrb_int joy;
  const char *name;
  mrb_get_args(mrb, "i", &joy);
  name = glfwGetJoystickName(joy);
  return mrb_str_new_cstr(mrb, name);
}

static mrb_value
glfw_cache_size(mrb_state *mrb, mrb_value self)
{
  return mrb_fixnum_value(RARRAY_LEN(mrb_glfw3_cache(mrb)));
}

void
mrb_mruby_glfw3_gem_init(mrb_state* mrb)
{
  struct RClass *glfw_module;
  /* Setup error callbacks */
  err_M = mrb;
  glfwSetErrorCallback(&glfw_error_func);
  mrb_define_class(mrb, "GLFWError", mrb_class_get(mrb, "StandardError"));
  /* GLFW module */
  glfw_module = mrb_define_module(mrb, "GLFW");
  /* Cache */
  mrb_iv_set(mrb, mrb_obj_value(glfw_module), mrb_intern_lit(mrb, "__glfw_objects"), mrb_ary_new(mrb));
  /* module methods */
  mrb_define_class_method(mrb, glfw_module, "init",                 glfw_init,                  MRB_ARGS_NONE());
  mrb_define_class_method(mrb, glfw_module, "terminate",            glfw_terminate,             MRB_ARGS_NONE());
  mrb_define_class_method(mrb, glfw_module, "version",              glfw_version,               MRB_ARGS_NONE());
  mrb_define_class_method(mrb, glfw_module, "version_string",       glfw_version_string,        MRB_ARGS_NONE());
  mrb_define_class_method(mrb, glfw_module, "default_window_hints", glfw_default_window_hints,  MRB_ARGS_NONE());
  mrb_define_class_method(mrb, glfw_module, "window_hint",          glfw_window_hint,           MRB_ARGS_REQ(2));
  mrb_define_class_method(mrb, glfw_module, "poll_events",          glfw_poll_events,           MRB_ARGS_NONE());
  mrb_define_class_method(mrb, glfw_module, "wait_events",          glfw_wait_events,           MRB_ARGS_NONE());
  mrb_define_class_method(mrb, glfw_module, "post_empty_event",     glfw_post_empty_event,      MRB_ARGS_NONE());
  mrb_define_class_method(mrb, glfw_module, "time",                 glfw_get_time,              MRB_ARGS_NONE());
  mrb_define_class_method(mrb, glfw_module, "time=",                glfw_set_time,              MRB_ARGS_REQ(1));
  mrb_define_class_method(mrb, glfw_module, "current_context",      glfw_current_context,       MRB_ARGS_NONE());
  mrb_define_class_method(mrb, glfw_module, "swap_interval=",       glfw_set_swap_interval,     MRB_ARGS_REQ(1));
  mrb_define_class_method(mrb, glfw_module, "extension_supported?", glfw_extension_supported_p, MRB_ARGS_REQ(1));
  mrb_define_class_method(mrb, glfw_module, "proc_address",         glfw_proc_address,          MRB_ARGS_REQ(1));
  /* Joystick */
  mrb_define_class_method(mrb, glfw_module, "joystick_present",     glfw_joystick_present,      MRB_ARGS_REQ(1));
  mrb_define_class_method(mrb, glfw_module, "joystick_axes",        glfw_joystick_axes,         MRB_ARGS_REQ(1));
  mrb_define_class_method(mrb, glfw_module, "joystick_buttons",     glfw_joystick_buttons,      MRB_ARGS_REQ(1));
  mrb_define_class_method(mrb, glfw_module, "joystick_name",        glfw_joystick_name,         MRB_ARGS_REQ(1));
  /* internal cache */
  mrb_define_class_method(mrb, glfw_module, "cache_size", glfw_cache_size, MRB_ARGS_NONE());
  /* Constants */
  mrb_define_const(mrb, glfw_module, "VERSION_MAJOR", mrb_fixnum_value(GLFW_VERSION_MAJOR));
  mrb_define_const(mrb, glfw_module, "VERSION_MINOR", mrb_fixnum_value(GLFW_VERSION_MINOR));
  mrb_define_const(mrb, glfw_module, "VERSION_REVISION", mrb_fixnum_value(GLFW_VERSION_REVISION));
  mrb_define_const(mrb, glfw_module, "RELEASE", mrb_fixnum_value(GLFW_RELEASE));
  mrb_define_const(mrb, glfw_module, "PRESS", mrb_fixnum_value(GLFW_PRESS));
  mrb_define_const(mrb, glfw_module, "REPEAT", mrb_fixnum_value(GLFW_REPEAT));
  mrb_define_const(mrb, glfw_module, "KEY_UNKNOWN", mrb_fixnum_value(GLFW_KEY_UNKNOWN));
  mrb_define_const(mrb, glfw_module, "KEY_SPACE", mrb_fixnum_value(GLFW_KEY_SPACE));
  mrb_define_const(mrb, glfw_module, "KEY_APOSTROPHE", mrb_fixnum_value(GLFW_KEY_APOSTROPHE));
  mrb_define_const(mrb, glfw_module, "KEY_COMMA", mrb_fixnum_value(GLFW_KEY_COMMA));
  mrb_define_const(mrb, glfw_module, "KEY_MINUS", mrb_fixnum_value(GLFW_KEY_MINUS));
  mrb_define_const(mrb, glfw_module, "KEY_PERIOD", mrb_fixnum_value(GLFW_KEY_PERIOD));
  mrb_define_const(mrb, glfw_module, "KEY_SLASH", mrb_fixnum_value(GLFW_KEY_SLASH));
  mrb_define_const(mrb, glfw_module, "KEY_0", mrb_fixnum_value(GLFW_KEY_0));
  mrb_define_const(mrb, glfw_module, "KEY_1", mrb_fixnum_value(GLFW_KEY_1));
  mrb_define_const(mrb, glfw_module, "KEY_2", mrb_fixnum_value(GLFW_KEY_2));
  mrb_define_const(mrb, glfw_module, "KEY_3", mrb_fixnum_value(GLFW_KEY_3));
  mrb_define_const(mrb, glfw_module, "KEY_4", mrb_fixnum_value(GLFW_KEY_4));
  mrb_define_const(mrb, glfw_module, "KEY_5", mrb_fixnum_value(GLFW_KEY_5));
  mrb_define_const(mrb, glfw_module, "KEY_6", mrb_fixnum_value(GLFW_KEY_6));
  mrb_define_const(mrb, glfw_module, "KEY_7", mrb_fixnum_value(GLFW_KEY_7));
  mrb_define_const(mrb, glfw_module, "KEY_8", mrb_fixnum_value(GLFW_KEY_8));
  mrb_define_const(mrb, glfw_module, "KEY_9", mrb_fixnum_value(GLFW_KEY_9));
  mrb_define_const(mrb, glfw_module, "KEY_SEMICOLON", mrb_fixnum_value(GLFW_KEY_SEMICOLON));
  mrb_define_const(mrb, glfw_module, "KEY_EQUAL", mrb_fixnum_value(GLFW_KEY_EQUAL));
  mrb_define_const(mrb, glfw_module, "KEY_A", mrb_fixnum_value(GLFW_KEY_A));
  mrb_define_const(mrb, glfw_module, "KEY_B", mrb_fixnum_value(GLFW_KEY_B));
  mrb_define_const(mrb, glfw_module, "KEY_C", mrb_fixnum_value(GLFW_KEY_C));
  mrb_define_const(mrb, glfw_module, "KEY_D", mrb_fixnum_value(GLFW_KEY_D));
  mrb_define_const(mrb, glfw_module, "KEY_E", mrb_fixnum_value(GLFW_KEY_E));
  mrb_define_const(mrb, glfw_module, "KEY_F", mrb_fixnum_value(GLFW_KEY_F));
  mrb_define_const(mrb, glfw_module, "KEY_G", mrb_fixnum_value(GLFW_KEY_G));
  mrb_define_const(mrb, glfw_module, "KEY_H", mrb_fixnum_value(GLFW_KEY_H));
  mrb_define_const(mrb, glfw_module, "KEY_I", mrb_fixnum_value(GLFW_KEY_I));
  mrb_define_const(mrb, glfw_module, "KEY_J", mrb_fixnum_value(GLFW_KEY_J));
  mrb_define_const(mrb, glfw_module, "KEY_K", mrb_fixnum_value(GLFW_KEY_K));
  mrb_define_const(mrb, glfw_module, "KEY_L", mrb_fixnum_value(GLFW_KEY_L));
  mrb_define_const(mrb, glfw_module, "KEY_M", mrb_fixnum_value(GLFW_KEY_M));
  mrb_define_const(mrb, glfw_module, "KEY_N", mrb_fixnum_value(GLFW_KEY_N));
  mrb_define_const(mrb, glfw_module, "KEY_O", mrb_fixnum_value(GLFW_KEY_O));
  mrb_define_const(mrb, glfw_module, "KEY_P", mrb_fixnum_value(GLFW_KEY_P));
  mrb_define_const(mrb, glfw_module, "KEY_Q", mrb_fixnum_value(GLFW_KEY_Q));
  mrb_define_const(mrb, glfw_module, "KEY_R", mrb_fixnum_value(GLFW_KEY_R));
  mrb_define_const(mrb, glfw_module, "KEY_S", mrb_fixnum_value(GLFW_KEY_S));
  mrb_define_const(mrb, glfw_module, "KEY_T", mrb_fixnum_value(GLFW_KEY_T));
  mrb_define_const(mrb, glfw_module, "KEY_U", mrb_fixnum_value(GLFW_KEY_U));
  mrb_define_const(mrb, glfw_module, "KEY_V", mrb_fixnum_value(GLFW_KEY_V));
  mrb_define_const(mrb, glfw_module, "KEY_W", mrb_fixnum_value(GLFW_KEY_W));
  mrb_define_const(mrb, glfw_module, "KEY_X", mrb_fixnum_value(GLFW_KEY_X));
  mrb_define_const(mrb, glfw_module, "KEY_Y", mrb_fixnum_value(GLFW_KEY_Y));
  mrb_define_const(mrb, glfw_module, "KEY_Z", mrb_fixnum_value(GLFW_KEY_Z));
  mrb_define_const(mrb, glfw_module, "KEY_LEFT_BRACKET", mrb_fixnum_value(GLFW_KEY_LEFT_BRACKET));
  mrb_define_const(mrb, glfw_module, "KEY_BACKSLASH", mrb_fixnum_value(GLFW_KEY_BACKSLASH));
  mrb_define_const(mrb, glfw_module, "KEY_RIGHT_BRACKET", mrb_fixnum_value(GLFW_KEY_RIGHT_BRACKET));
  mrb_define_const(mrb, glfw_module, "KEY_GRAVE_ACCENT", mrb_fixnum_value(GLFW_KEY_GRAVE_ACCENT));
  mrb_define_const(mrb, glfw_module, "KEY_WORLD_1", mrb_fixnum_value(GLFW_KEY_WORLD_1));
  mrb_define_const(mrb, glfw_module, "KEY_WORLD_2", mrb_fixnum_value(GLFW_KEY_WORLD_2));
  mrb_define_const(mrb, glfw_module, "KEY_ESCAPE", mrb_fixnum_value(GLFW_KEY_ESCAPE));
  mrb_define_const(mrb, glfw_module, "KEY_ENTER", mrb_fixnum_value(GLFW_KEY_ENTER));
  mrb_define_const(mrb, glfw_module, "KEY_TAB", mrb_fixnum_value(GLFW_KEY_TAB));
  mrb_define_const(mrb, glfw_module, "KEY_BACKSPACE", mrb_fixnum_value(GLFW_KEY_BACKSPACE));
  mrb_define_const(mrb, glfw_module, "KEY_INSERT", mrb_fixnum_value(GLFW_KEY_INSERT));
  mrb_define_const(mrb, glfw_module, "KEY_DELETE", mrb_fixnum_value(GLFW_KEY_DELETE));
  mrb_define_const(mrb, glfw_module, "KEY_RIGHT", mrb_fixnum_value(GLFW_KEY_RIGHT));
  mrb_define_const(mrb, glfw_module, "KEY_LEFT", mrb_fixnum_value(GLFW_KEY_LEFT));
  mrb_define_const(mrb, glfw_module, "KEY_DOWN", mrb_fixnum_value(GLFW_KEY_DOWN));
  mrb_define_const(mrb, glfw_module, "KEY_UP", mrb_fixnum_value(GLFW_KEY_UP));
  mrb_define_const(mrb, glfw_module, "KEY_PAGE_UP", mrb_fixnum_value(GLFW_KEY_PAGE_UP));
  mrb_define_const(mrb, glfw_module, "KEY_PAGE_DOWN", mrb_fixnum_value(GLFW_KEY_PAGE_DOWN));
  mrb_define_const(mrb, glfw_module, "KEY_HOME", mrb_fixnum_value(GLFW_KEY_HOME));
  mrb_define_const(mrb, glfw_module, "KEY_END", mrb_fixnum_value(GLFW_KEY_END));
  mrb_define_const(mrb, glfw_module, "KEY_CAPS_LOCK", mrb_fixnum_value(GLFW_KEY_CAPS_LOCK));
  mrb_define_const(mrb, glfw_module, "KEY_SCROLL_LOCK", mrb_fixnum_value(GLFW_KEY_SCROLL_LOCK));
  mrb_define_const(mrb, glfw_module, "KEY_NUM_LOCK", mrb_fixnum_value(GLFW_KEY_NUM_LOCK));
  mrb_define_const(mrb, glfw_module, "KEY_PRINT_SCREEN", mrb_fixnum_value(GLFW_KEY_PRINT_SCREEN));
  mrb_define_const(mrb, glfw_module, "KEY_PAUSE", mrb_fixnum_value(GLFW_KEY_PAUSE));
  mrb_define_const(mrb, glfw_module, "KEY_F1", mrb_fixnum_value(GLFW_KEY_F1));
  mrb_define_const(mrb, glfw_module, "KEY_F2", mrb_fixnum_value(GLFW_KEY_F2));
  mrb_define_const(mrb, glfw_module, "KEY_F3", mrb_fixnum_value(GLFW_KEY_F3));
  mrb_define_const(mrb, glfw_module, "KEY_F4", mrb_fixnum_value(GLFW_KEY_F4));
  mrb_define_const(mrb, glfw_module, "KEY_F5", mrb_fixnum_value(GLFW_KEY_F5));
  mrb_define_const(mrb, glfw_module, "KEY_F6", mrb_fixnum_value(GLFW_KEY_F6));
  mrb_define_const(mrb, glfw_module, "KEY_F7", mrb_fixnum_value(GLFW_KEY_F7));
  mrb_define_const(mrb, glfw_module, "KEY_F8", mrb_fixnum_value(GLFW_KEY_F8));
  mrb_define_const(mrb, glfw_module, "KEY_F9", mrb_fixnum_value(GLFW_KEY_F9));
  mrb_define_const(mrb, glfw_module, "KEY_F10", mrb_fixnum_value(GLFW_KEY_F10));
  mrb_define_const(mrb, glfw_module, "KEY_F11", mrb_fixnum_value(GLFW_KEY_F11));
  mrb_define_const(mrb, glfw_module, "KEY_F12", mrb_fixnum_value(GLFW_KEY_F12));
  mrb_define_const(mrb, glfw_module, "KEY_F13", mrb_fixnum_value(GLFW_KEY_F13));
  mrb_define_const(mrb, glfw_module, "KEY_F14", mrb_fixnum_value(GLFW_KEY_F14));
  mrb_define_const(mrb, glfw_module, "KEY_F15", mrb_fixnum_value(GLFW_KEY_F15));
  mrb_define_const(mrb, glfw_module, "KEY_F16", mrb_fixnum_value(GLFW_KEY_F16));
  mrb_define_const(mrb, glfw_module, "KEY_F17", mrb_fixnum_value(GLFW_KEY_F17));
  mrb_define_const(mrb, glfw_module, "KEY_F18", mrb_fixnum_value(GLFW_KEY_F18));
  mrb_define_const(mrb, glfw_module, "KEY_F19", mrb_fixnum_value(GLFW_KEY_F19));
  mrb_define_const(mrb, glfw_module, "KEY_F20", mrb_fixnum_value(GLFW_KEY_F20));
  mrb_define_const(mrb, glfw_module, "KEY_F21", mrb_fixnum_value(GLFW_KEY_F21));
  mrb_define_const(mrb, glfw_module, "KEY_F22", mrb_fixnum_value(GLFW_KEY_F22));
  mrb_define_const(mrb, glfw_module, "KEY_F23", mrb_fixnum_value(GLFW_KEY_F23));
  mrb_define_const(mrb, glfw_module, "KEY_F24", mrb_fixnum_value(GLFW_KEY_F24));
  mrb_define_const(mrb, glfw_module, "KEY_F25", mrb_fixnum_value(GLFW_KEY_F25));
  mrb_define_const(mrb, glfw_module, "KEY_KP_0", mrb_fixnum_value(GLFW_KEY_KP_0));
  mrb_define_const(mrb, glfw_module, "KEY_KP_1", mrb_fixnum_value(GLFW_KEY_KP_1));
  mrb_define_const(mrb, glfw_module, "KEY_KP_2", mrb_fixnum_value(GLFW_KEY_KP_2));
  mrb_define_const(mrb, glfw_module, "KEY_KP_3", mrb_fixnum_value(GLFW_KEY_KP_3));
  mrb_define_const(mrb, glfw_module, "KEY_KP_4", mrb_fixnum_value(GLFW_KEY_KP_4));
  mrb_define_const(mrb, glfw_module, "KEY_KP_5", mrb_fixnum_value(GLFW_KEY_KP_5));
  mrb_define_const(mrb, glfw_module, "KEY_KP_6", mrb_fixnum_value(GLFW_KEY_KP_6));
  mrb_define_const(mrb, glfw_module, "KEY_KP_7", mrb_fixnum_value(GLFW_KEY_KP_7));
  mrb_define_const(mrb, glfw_module, "KEY_KP_8", mrb_fixnum_value(GLFW_KEY_KP_8));
  mrb_define_const(mrb, glfw_module, "KEY_KP_9", mrb_fixnum_value(GLFW_KEY_KP_9));
  mrb_define_const(mrb, glfw_module, "KEY_KP_DECIMAL", mrb_fixnum_value(GLFW_KEY_KP_DECIMAL));
  mrb_define_const(mrb, glfw_module, "KEY_KP_DIVIDE", mrb_fixnum_value(GLFW_KEY_KP_DIVIDE));
  mrb_define_const(mrb, glfw_module, "KEY_KP_MULTIPLY", mrb_fixnum_value(GLFW_KEY_KP_MULTIPLY));
  mrb_define_const(mrb, glfw_module, "KEY_KP_SUBTRACT", mrb_fixnum_value(GLFW_KEY_KP_SUBTRACT));
  mrb_define_const(mrb, glfw_module, "KEY_KP_ADD", mrb_fixnum_value(GLFW_KEY_KP_ADD));
  mrb_define_const(mrb, glfw_module, "KEY_KP_ENTER", mrb_fixnum_value(GLFW_KEY_KP_ENTER));
  mrb_define_const(mrb, glfw_module, "KEY_KP_EQUAL", mrb_fixnum_value(GLFW_KEY_KP_EQUAL));
  mrb_define_const(mrb, glfw_module, "KEY_LEFT_SHIFT", mrb_fixnum_value(GLFW_KEY_LEFT_SHIFT));
  mrb_define_const(mrb, glfw_module, "KEY_LEFT_CONTROL", mrb_fixnum_value(GLFW_KEY_LEFT_CONTROL));
  mrb_define_const(mrb, glfw_module, "KEY_LEFT_ALT", mrb_fixnum_value(GLFW_KEY_LEFT_ALT));
  mrb_define_const(mrb, glfw_module, "KEY_LEFT_SUPER", mrb_fixnum_value(GLFW_KEY_LEFT_SUPER));
  mrb_define_const(mrb, glfw_module, "KEY_RIGHT_SHIFT", mrb_fixnum_value(GLFW_KEY_RIGHT_SHIFT));
  mrb_define_const(mrb, glfw_module, "KEY_RIGHT_CONTROL", mrb_fixnum_value(GLFW_KEY_RIGHT_CONTROL));
  mrb_define_const(mrb, glfw_module, "KEY_RIGHT_ALT", mrb_fixnum_value(GLFW_KEY_RIGHT_ALT));
  mrb_define_const(mrb, glfw_module, "KEY_RIGHT_SUPER", mrb_fixnum_value(GLFW_KEY_RIGHT_SUPER));
  mrb_define_const(mrb, glfw_module, "KEY_MENU", mrb_fixnum_value(GLFW_KEY_MENU));
  mrb_define_const(mrb, glfw_module, "KEY_LAST", mrb_fixnum_value(GLFW_KEY_LAST));
  mrb_define_const(mrb, glfw_module, "MOD_SHIFT", mrb_fixnum_value(GLFW_MOD_SHIFT));
  mrb_define_const(mrb, glfw_module, "MOD_CONTROL", mrb_fixnum_value(GLFW_MOD_CONTROL));
  mrb_define_const(mrb, glfw_module, "MOD_ALT", mrb_fixnum_value(GLFW_MOD_ALT));
  mrb_define_const(mrb, glfw_module, "MOD_SUPER", mrb_fixnum_value(GLFW_MOD_SUPER));
  mrb_define_const(mrb, glfw_module, "MOUSE_BUTTON_1", mrb_fixnum_value(GLFW_MOUSE_BUTTON_1));
  mrb_define_const(mrb, glfw_module, "MOUSE_BUTTON_2", mrb_fixnum_value(GLFW_MOUSE_BUTTON_2));
  mrb_define_const(mrb, glfw_module, "MOUSE_BUTTON_3", mrb_fixnum_value(GLFW_MOUSE_BUTTON_3));
  mrb_define_const(mrb, glfw_module, "MOUSE_BUTTON_4", mrb_fixnum_value(GLFW_MOUSE_BUTTON_4));
  mrb_define_const(mrb, glfw_module, "MOUSE_BUTTON_5", mrb_fixnum_value(GLFW_MOUSE_BUTTON_5));
  mrb_define_const(mrb, glfw_module, "MOUSE_BUTTON_6", mrb_fixnum_value(GLFW_MOUSE_BUTTON_6));
  mrb_define_const(mrb, glfw_module, "MOUSE_BUTTON_7", mrb_fixnum_value(GLFW_MOUSE_BUTTON_7));
  mrb_define_const(mrb, glfw_module, "MOUSE_BUTTON_8", mrb_fixnum_value(GLFW_MOUSE_BUTTON_8));
  mrb_define_const(mrb, glfw_module, "MOUSE_BUTTON_LAST", mrb_fixnum_value(GLFW_MOUSE_BUTTON_LAST));
  mrb_define_const(mrb, glfw_module, "MOUSE_BUTTON_LEFT", mrb_fixnum_value(GLFW_MOUSE_BUTTON_LEFT));
  mrb_define_const(mrb, glfw_module, "MOUSE_BUTTON_RIGHT", mrb_fixnum_value(GLFW_MOUSE_BUTTON_RIGHT));
  mrb_define_const(mrb, glfw_module, "MOUSE_BUTTON_MIDDLE", mrb_fixnum_value(GLFW_MOUSE_BUTTON_MIDDLE));
  mrb_define_const(mrb, glfw_module, "JOYSTICK_1", mrb_fixnum_value(GLFW_JOYSTICK_1));
  mrb_define_const(mrb, glfw_module, "JOYSTICK_2", mrb_fixnum_value(GLFW_JOYSTICK_2));
  mrb_define_const(mrb, glfw_module, "JOYSTICK_3", mrb_fixnum_value(GLFW_JOYSTICK_3));
  mrb_define_const(mrb, glfw_module, "JOYSTICK_4", mrb_fixnum_value(GLFW_JOYSTICK_4));
  mrb_define_const(mrb, glfw_module, "JOYSTICK_5", mrb_fixnum_value(GLFW_JOYSTICK_5));
  mrb_define_const(mrb, glfw_module, "JOYSTICK_6", mrb_fixnum_value(GLFW_JOYSTICK_6));
  mrb_define_const(mrb, glfw_module, "JOYSTICK_7", mrb_fixnum_value(GLFW_JOYSTICK_7));
  mrb_define_const(mrb, glfw_module, "JOYSTICK_8", mrb_fixnum_value(GLFW_JOYSTICK_8));
  mrb_define_const(mrb, glfw_module, "JOYSTICK_9", mrb_fixnum_value(GLFW_JOYSTICK_9));
  mrb_define_const(mrb, glfw_module, "JOYSTICK_10", mrb_fixnum_value(GLFW_JOYSTICK_10));
  mrb_define_const(mrb, glfw_module, "JOYSTICK_11", mrb_fixnum_value(GLFW_JOYSTICK_11));
  mrb_define_const(mrb, glfw_module, "JOYSTICK_12", mrb_fixnum_value(GLFW_JOYSTICK_12));
  mrb_define_const(mrb, glfw_module, "JOYSTICK_13", mrb_fixnum_value(GLFW_JOYSTICK_13));
  mrb_define_const(mrb, glfw_module, "JOYSTICK_14", mrb_fixnum_value(GLFW_JOYSTICK_14));
  mrb_define_const(mrb, glfw_module, "JOYSTICK_15", mrb_fixnum_value(GLFW_JOYSTICK_15));
  mrb_define_const(mrb, glfw_module, "JOYSTICK_16", mrb_fixnum_value(GLFW_JOYSTICK_16));
  mrb_define_const(mrb, glfw_module, "JOYSTICK_LAST", mrb_fixnum_value(GLFW_JOYSTICK_LAST));
  mrb_define_const(mrb, glfw_module, "NOT_INITIALIZED", mrb_fixnum_value(GLFW_NOT_INITIALIZED));
  mrb_define_const(mrb, glfw_module, "NO_CURRENT_CONTEXT", mrb_fixnum_value(GLFW_NO_CURRENT_CONTEXT));
  mrb_define_const(mrb, glfw_module, "INVALID_ENUM", mrb_fixnum_value(GLFW_INVALID_ENUM));
  mrb_define_const(mrb, glfw_module, "INVALID_VALUE", mrb_fixnum_value(GLFW_INVALID_VALUE));
  mrb_define_const(mrb, glfw_module, "OUT_OF_MEMORY", mrb_fixnum_value(GLFW_OUT_OF_MEMORY));
  mrb_define_const(mrb, glfw_module, "API_UNAVAILABLE", mrb_fixnum_value(GLFW_API_UNAVAILABLE));
  mrb_define_const(mrb, glfw_module, "VERSION_UNAVAILABLE", mrb_fixnum_value(GLFW_VERSION_UNAVAILABLE));
  mrb_define_const(mrb, glfw_module, "PLATFORM_ERROR", mrb_fixnum_value(GLFW_PLATFORM_ERROR));
  mrb_define_const(mrb, glfw_module, "FORMAT_UNAVAILABLE", mrb_fixnum_value(GLFW_FORMAT_UNAVAILABLE));
  mrb_define_const(mrb, glfw_module, "FOCUSED", mrb_fixnum_value(GLFW_FOCUSED));
  mrb_define_const(mrb, glfw_module, "ICONIFIED", mrb_fixnum_value(GLFW_ICONIFIED));
  mrb_define_const(mrb, glfw_module, "RESIZABLE", mrb_fixnum_value(GLFW_RESIZABLE));
  mrb_define_const(mrb, glfw_module, "VISIBLE", mrb_fixnum_value(GLFW_VISIBLE));
  mrb_define_const(mrb, glfw_module, "DECORATED", mrb_fixnum_value(GLFW_DECORATED));
  mrb_define_const(mrb, glfw_module, "AUTO_ICONIFY", mrb_fixnum_value(GLFW_AUTO_ICONIFY));
  mrb_define_const(mrb, glfw_module, "FLOATING", mrb_fixnum_value(GLFW_FLOATING));
  mrb_define_const(mrb, glfw_module, "RED_BITS", mrb_fixnum_value(GLFW_RED_BITS));
  mrb_define_const(mrb, glfw_module, "GREEN_BITS", mrb_fixnum_value(GLFW_GREEN_BITS));
  mrb_define_const(mrb, glfw_module, "BLUE_BITS", mrb_fixnum_value(GLFW_BLUE_BITS));
  mrb_define_const(mrb, glfw_module, "ALPHA_BITS", mrb_fixnum_value(GLFW_ALPHA_BITS));
  mrb_define_const(mrb, glfw_module, "DEPTH_BITS", mrb_fixnum_value(GLFW_DEPTH_BITS));
  mrb_define_const(mrb, glfw_module, "STENCIL_BITS", mrb_fixnum_value(GLFW_STENCIL_BITS));
  mrb_define_const(mrb, glfw_module, "ACCUM_RED_BITS", mrb_fixnum_value(GLFW_ACCUM_RED_BITS));
  mrb_define_const(mrb, glfw_module, "ACCUM_GREEN_BITS", mrb_fixnum_value(GLFW_ACCUM_GREEN_BITS));
  mrb_define_const(mrb, glfw_module, "ACCUM_BLUE_BITS", mrb_fixnum_value(GLFW_ACCUM_BLUE_BITS));
  mrb_define_const(mrb, glfw_module, "ACCUM_ALPHA_BITS", mrb_fixnum_value(GLFW_ACCUM_ALPHA_BITS));
  mrb_define_const(mrb, glfw_module, "AUX_BUFFERS", mrb_fixnum_value(GLFW_AUX_BUFFERS));
  mrb_define_const(mrb, glfw_module, "STEREO", mrb_fixnum_value(GLFW_STEREO));
  mrb_define_const(mrb, glfw_module, "SAMPLES", mrb_fixnum_value(GLFW_SAMPLES));
  mrb_define_const(mrb, glfw_module, "SRGB_CAPABLE", mrb_fixnum_value(GLFW_SRGB_CAPABLE));
  mrb_define_const(mrb, glfw_module, "REFRESH_RATE", mrb_fixnum_value(GLFW_REFRESH_RATE));
  mrb_define_const(mrb, glfw_module, "DOUBLEBUFFER", mrb_fixnum_value(GLFW_DOUBLEBUFFER));
  mrb_define_const(mrb, glfw_module, "CLIENT_API", mrb_fixnum_value(GLFW_CLIENT_API));
  mrb_define_const(mrb, glfw_module, "CONTEXT_VERSION_MAJOR", mrb_fixnum_value(GLFW_CONTEXT_VERSION_MAJOR));
  mrb_define_const(mrb, glfw_module, "CONTEXT_VERSION_MINOR", mrb_fixnum_value(GLFW_CONTEXT_VERSION_MINOR));
  mrb_define_const(mrb, glfw_module, "CONTEXT_REVISION", mrb_fixnum_value(GLFW_CONTEXT_REVISION));
  mrb_define_const(mrb, glfw_module, "CONTEXT_ROBUSTNESS", mrb_fixnum_value(GLFW_CONTEXT_ROBUSTNESS));
  mrb_define_const(mrb, glfw_module, "OPENGL_FORWARD_COMPAT", mrb_fixnum_value(GLFW_OPENGL_FORWARD_COMPAT));
  mrb_define_const(mrb, glfw_module, "OPENGL_DEBUG_CONTEXT", mrb_fixnum_value(GLFW_OPENGL_DEBUG_CONTEXT));
  mrb_define_const(mrb, glfw_module, "OPENGL_PROFILE", mrb_fixnum_value(GLFW_OPENGL_PROFILE));
  mrb_define_const(mrb, glfw_module, "CONTEXT_RELEASE_BEHAVIOR", mrb_fixnum_value(GLFW_CONTEXT_RELEASE_BEHAVIOR));
  mrb_define_const(mrb, glfw_module, "OPENGL_API", mrb_fixnum_value(GLFW_OPENGL_API));
  mrb_define_const(mrb, glfw_module, "OPENGL_ES_API", mrb_fixnum_value(GLFW_OPENGL_ES_API));
  mrb_define_const(mrb, glfw_module, "NO_ROBUSTNESS", mrb_fixnum_value(GLFW_NO_ROBUSTNESS));
  mrb_define_const(mrb, glfw_module, "NO_RESET_NOTIFICATION", mrb_fixnum_value(GLFW_NO_RESET_NOTIFICATION));
  mrb_define_const(mrb, glfw_module, "LOSE_CONTEXT_ON_RESET", mrb_fixnum_value(GLFW_LOSE_CONTEXT_ON_RESET));
  mrb_define_const(mrb, glfw_module, "OPENGL_ANY_PROFILE", mrb_fixnum_value(GLFW_OPENGL_ANY_PROFILE));
  mrb_define_const(mrb, glfw_module, "OPENGL_CORE_PROFILE", mrb_fixnum_value(GLFW_OPENGL_CORE_PROFILE));
  mrb_define_const(mrb, glfw_module, "OPENGL_COMPAT_PROFILE", mrb_fixnum_value(GLFW_OPENGL_COMPAT_PROFILE));
  mrb_define_const(mrb, glfw_module, "CURSOR", mrb_fixnum_value(GLFW_CURSOR));
  mrb_define_const(mrb, glfw_module, "STICKY_KEYS", mrb_fixnum_value(GLFW_STICKY_KEYS));
  mrb_define_const(mrb, glfw_module, "STICKY_MOUSE_BUTTONS", mrb_fixnum_value(GLFW_STICKY_MOUSE_BUTTONS));
  mrb_define_const(mrb, glfw_module, "CURSOR_NORMAL", mrb_fixnum_value(GLFW_CURSOR_NORMAL));
  mrb_define_const(mrb, glfw_module, "CURSOR_HIDDEN", mrb_fixnum_value(GLFW_CURSOR_HIDDEN));
  mrb_define_const(mrb, glfw_module, "CURSOR_DISABLED", mrb_fixnum_value(GLFW_CURSOR_DISABLED));
  mrb_define_const(mrb, glfw_module, "ANY_RELEASE_BEHAVIOR", mrb_fixnum_value(GLFW_ANY_RELEASE_BEHAVIOR));
  mrb_define_const(mrb, glfw_module, "RELEASE_BEHAVIOR_FLUSH", mrb_fixnum_value(GLFW_RELEASE_BEHAVIOR_FLUSH));
  mrb_define_const(mrb, glfw_module, "RELEASE_BEHAVIOR_NONE", mrb_fixnum_value(GLFW_RELEASE_BEHAVIOR_NONE));
  mrb_define_const(mrb, glfw_module, "ARROW_CURSOR", mrb_fixnum_value(GLFW_ARROW_CURSOR));
  mrb_define_const(mrb, glfw_module, "IBEAM_CURSOR", mrb_fixnum_value(GLFW_IBEAM_CURSOR));
  mrb_define_const(mrb, glfw_module, "CROSSHAIR_CURSOR", mrb_fixnum_value(GLFW_CROSSHAIR_CURSOR));
  mrb_define_const(mrb, glfw_module, "HAND_CURSOR", mrb_fixnum_value(GLFW_HAND_CURSOR));
  mrb_define_const(mrb, glfw_module, "HRESIZE_CURSOR", mrb_fixnum_value(GLFW_HRESIZE_CURSOR));
  mrb_define_const(mrb, glfw_module, "VRESIZE_CURSOR", mrb_fixnum_value(GLFW_VRESIZE_CURSOR));
  mrb_define_const(mrb, glfw_module, "CONNECTED", mrb_fixnum_value(GLFW_CONNECTED));
  mrb_define_const(mrb, glfw_module, "DISCONNECTED", mrb_fixnum_value(GLFW_DISCONNECTED));
  mrb_define_const(mrb, glfw_module, "DONT_CARE", mrb_fixnum_value(GLFW_DONT_CARE));
  /* sub-modules */
  mrb_glfw3_vid_mode_init(mrb, glfw_module);
  mrb_glfw3_gamma_ramp_init(mrb, glfw_module);
  mrb_glfw3_image_init(mrb, glfw_module);
  mrb_glfw3_cursor_init(mrb, glfw_module);
  mrb_glfw3_monitor_init(mrb, glfw_module);
  mrb_glfw3_window_init(mrb, glfw_module);
}

void
mrb_mruby_glfw3_gem_final(mrb_state *mrb)
{
  glfw_terminate_m(mrb);
  err_M = NULL;
}
