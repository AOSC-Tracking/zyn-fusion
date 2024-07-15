#include <stdbool.h>

#include <mruby.h>
#include <mruby/array.h>
#include <mruby/class.h>
#include <mruby/data.h>
#include <mruby/hash.h>
#include <mruby/variable.h>
#include <mruby/string.h>

#include <GLFW/glfw3.h>

#include "glfw3_private.h"
#include "glfw3_window.h"
#include "glfw3_monitor.h"

/* START THE HAX */
typedef unsigned int uint;
#define uint_cast(_mrb_, a) mrb_fixnum_value(a)
#define int_cast(_mrb_, a) mrb_fixnum_value(a)
#define float_cast(_mrb_, a) mrb_float_value(_mrb_, a)
#define double_cast(_mrb_, a) mrb_float_value(_mrb_, a)
#define string_cast(_mrb_, a) mrb_str_new_cstr(_mrb_, a)
#define to_cast(name) name ## _cast
#define CALLBACK_IDENT(_base_) window_ ## _base_ ## _func
#define CALLBACK_NAME(_base_) "window_" #_base_ "_func"
#define GET_CALLBACK(_func_) mrb_iv_get(cb_MRB, mrb_window, mrb_intern_lit(cb_MRB, CALLBACK_NAME(_func_)))
#define GET_WINDOW_REF(_mrb_, window) mrb_obj_value(glfwGetWindowUserPointer(window))

#define MAKE_MRB_CALLBACK(_name_, _func_) \
static mrb_value                                                          \
window_set_ ## _func_ ## _callback(mrb_state *mrb, mrb_value self)        \
{                                                                         \
  mrb_value blk;                                                          \
  mrb_get_args(mrb, "&", &blk);                                           \
  if (mrb_nil_p(blk)) {                                                   \
    _name_(get_window(mrb, self), NULL);                                  \
  } else {                                                                \
    _name_(get_window(mrb, self), CALLBACK_IDENT(_func_));                 \
  }                                                                       \
  mrb_iv_set(mrb, self, mrb_intern_lit(mrb, CALLBACK_NAME(_func_)), blk); \
  return blk;                                                             \
}

#define CALLBACK_SETUP_N0(_name_, _func_) \
static void CALLBACK_IDENT(_func_)(GLFWwindow *window) { \
  mrb_value argv[1];           \
  mrb_value mrb_window = GET_WINDOW_REF(cb_MRB, window); \
  argv[0] = mrb_window;        \
  mrb_yield_argv(cb_MRB, GET_CALLBACK(_func_), 1, argv); \
} \
MAKE_MRB_CALLBACK(_name_, _func_);

#define CALLBACK_SETUP_N1(_name_, _func_, _t0_) \
static void CALLBACK_IDENT(_func_)(GLFWwindow *window, _t0_ p0) { \
  mrb_value argv[2];           \
  mrb_value mrb_window = GET_WINDOW_REF(cb_MRB, window); \
  argv[0] = mrb_window;    \
  argv[1] = to_cast(_t0_)(cb_MRB, p0); \
  mrb_yield_argv(cb_MRB, GET_CALLBACK(_func_), 2, argv); \
} \
MAKE_MRB_CALLBACK(_name_, _func_);

#define CALLBACK_SETUP_N2(_name_, _func_, _t0_, _t1_) \
static void CALLBACK_IDENT(_func_)(GLFWwindow *window, _t0_ p0, _t1_ p1) { \
  mrb_value argv[3];     \
  mrb_value mrb_window = GET_WINDOW_REF(cb_MRB, window); \
  argv[0] = mrb_window;     \
  argv[1] = to_cast(_t0_)(cb_MRB, p0); \
  argv[2] = to_cast(_t1_)(cb_MRB, p1); \
  mrb_yield_argv(cb_MRB, GET_CALLBACK(_func_), 3, argv); \
} \
MAKE_MRB_CALLBACK(_name_, _func_);

#define CALLBACK_SETUP_N3(_name_, _func_, _t0_, _t1_, _t2_) \
static void CALLBACK_IDENT(_func_)(GLFWwindow *window, _t0_ p0, _t1_ p1, _t2_ p2) { \
  mrb_value argv[4];     \
  mrb_value mrb_window = GET_WINDOW_REF(cb_MRB, window); \
  argv[0] = mrb_window;     \
  argv[1] = to_cast(_t0_)(cb_MRB, p0); \
  argv[2] = to_cast(_t1_)(cb_MRB, p1); \
  argv[3] = to_cast(_t2_)(cb_MRB, p2); \
  mrb_yield_argv(cb_MRB, GET_CALLBACK(_func_), 4, argv); \
} \
MAKE_MRB_CALLBACK(_name_, _func_);

#define CALLBACK_SETUP_N4(_name_, _func_, _t0_, _t1_, _t2_, _t3_) \
static void CALLBACK_IDENT(_func_)(GLFWwindow *window, _t0_ p0, _t1_ p1, _t2_ p2, _t3_ p3) { \
  mrb_value argv[5];     \
  mrb_value mrb_window = GET_WINDOW_REF(cb_MRB, window); \
  argv[0] = mrb_window;     \
  argv[1] = to_cast(_t0_)(cb_MRB, p0); \
  argv[2] = to_cast(_t1_)(cb_MRB, p1); \
  argv[3] = to_cast(_t2_)(cb_MRB, p2); \
  argv[4] = to_cast(_t3_)(cb_MRB, p3); \
  mrb_yield_argv(cb_MRB, GET_CALLBACK(_func_), 5, argv); \
} \
MAKE_MRB_CALLBACK(_name_, _func_);

#define CALLBACK_SETUP_N_ary(_name_, _func_, _t_, _cast_) \
static void CALLBACK_IDENT(_func_)(GLFWwindow *window, int size, _t_ p0) { \
  mrb_value argv[2]; \
  mrb_value data; \
  int i; \
  mrb_value mrb_window = GET_WINDOW_REF(cb_MRB, window); \
  data = mrb_ary_new(cb_MRB); \
  for (i = 0; i < size; ++i) { \
    mrb_ary_push(cb_MRB, data, to_cast(_cast_)(cb_MRB, p0[i])); \
  } \
  argv[0] = mrb_window; \
  argv[1] = data; \
  mrb_yield_argv(cb_MRB, GET_CALLBACK(_func_), 2, argv); \
} \
MAKE_MRB_CALLBACK(_name_, _func_);
/* END OF HAX */


static struct RClass *mrb_glfw3_window_class;
static mrb_state *cb_MRB;

void
window_free(mrb_state *mrb, void *ptr)
{
  if (ptr) {
    glfwDestroyWindow((GLFWwindow*)ptr);
  }
}

const struct mrb_data_type mrb_glfw3_window_type = { "GLFWwindow", window_free };

static inline GLFWwindow*
get_window(mrb_state *mrb, mrb_value self)
{
  return (GLFWwindow*)mrb_data_get_ptr(mrb, self, &mrb_glfw3_window_type);
}

static mrb_value
window_initialize(mrb_state *mrb, mrb_value self)
{
  mrb_int w, h;
  char *title;
  GLFWwindow *win;
  mrb_value monitor = mrb_nil_value(), share = mrb_nil_value();
  mrb_get_args(mrb, "iiz|oo", &w, &h, &title, &monitor, &share);
  win = glfwCreateWindow(w, h, title, NULL, NULL);
  if (!win) {
    mrb_raise(mrb, E_GLFW_ERROR, "Could not create Window.");
  }
  DATA_PTR(self) = win;
  DATA_TYPE(self) = &mrb_glfw3_window_type;
  glfwSetWindowUserPointer(win, mrb_obj_ptr(self));
  mrb_glfw3_cache_object(mrb, self);
  return self;
}

static mrb_value
window_destroy(mrb_state *mrb, mrb_value self)
{
  window_free(mrb, get_window(mrb, self));
  DATA_PTR(self) = NULL;
  DATA_TYPE(self) = NULL;
  mrb_glfw3_uncache_object(mrb, self);
  return mrb_nil_value();
}

static mrb_value
window_make_current(mrb_state *M, mrb_value self)
{
  return glfwMakeContextCurrent(get_window(M, self)), self;
}

static mrb_value
window_swap_buffers(mrb_state *M, mrb_value self)
{
  return glfwSwapBuffers(get_window(M, self)), self;
}

static mrb_value
window_get_clipboard(mrb_state *M, mrb_value self)
{
  return mrb_str_new_cstr(M, glfwGetClipboardString(get_window(M, self)));
}

static mrb_value
window_set_clipboard(mrb_state *M, mrb_value self)
{
  char* str;
  mrb_get_args(M, "z", &str);
  glfwSetClipboardString(get_window(M, self), str);
  return mrb_nil_value();
}

static mrb_value
window_set_title(mrb_state *M, mrb_value self)
{
  char* str;
  mrb_get_args(M, "z", &str);
  glfwSetWindowTitle(get_window(M, self), str);
  return mrb_nil_value();
}

static mrb_value
window_get_should_close(mrb_state *mrb, mrb_value self)
{
  return mrb_bool_value(glfwWindowShouldClose(get_window(mrb, self)));
}

static mrb_value
window_set_should_close(mrb_state *mrb, mrb_value self)
{
  mrb_int b;
  mrb_get_args(mrb, "i", &b);
  glfwSetWindowShouldClose(get_window(mrb, self), b);
  return mrb_nil_value();
}

static mrb_value
window_size_get(mrb_state *M, mrb_value self)
{
  int w, h;
  mrb_value ret[2];
  glfwGetWindowSize(get_window(M, self), &w, &h);
  ret[0] = mrb_fixnum_value(w);
  ret[1] = mrb_fixnum_value(h);
  return mrb_ary_new_from_values(M, 2, ret);
}

static mrb_value
window_size_set(mrb_state *M, mrb_value self)
{
  mrb_value ary;
  mrb_get_args(M, "A", &ary);
  glfwSetWindowSize(get_window(M, self),
                    mrb_int(M, mrb_ary_entry(ary, 0)),
                    mrb_int(M, mrb_ary_entry(ary, 1)));
  return ary;
}

static mrb_value
window_pos_get(mrb_state *M, mrb_value self)
{
  int w, h;
  mrb_value ret[2];
  glfwGetWindowPos(get_window(M, self), &w, &h);
  ret[0] = mrb_fixnum_value(w);
  ret[1] = mrb_fixnum_value(h);
  return mrb_ary_new_from_values(M, 2, ret);
}

static mrb_value
window_pos_set(mrb_state *M, mrb_value self)
{
  mrb_value ary;
  mrb_get_args(M, "A", &ary);
  glfwSetWindowPos(get_window(M, self),
                   mrb_int(M, mrb_ary_entry(ary, 0)),
                   mrb_int(M, mrb_ary_entry(ary, 1)));
  return ary;
}

static mrb_value
window_cursor_pos_get(mrb_state *mrb, mrb_value self)
{
  double x, y;
  mrb_value ret[2];
  glfwGetCursorPos(get_window(mrb, self), &x, &y);
  ret[0] = mrb_float_value(mrb, x);
  ret[1] = mrb_float_value(mrb, y);
  return mrb_ary_new_from_values(mrb, 2, ret);
}

static mrb_value
window_cursor_pos_set(mrb_state *M, mrb_value self)
{
  mrb_value ary;
  mrb_get_args(M, "A", &ary);
  glfwSetWindowPos(get_window(M, self),
                   mrb_float(mrb_ary_entry(ary, 0)),
                   mrb_float(mrb_ary_entry(ary, 1)));
  return ary;
}

static mrb_value
window_framebuffer_size(mrb_state *M, mrb_value self)
{
  int w, h;
  mrb_value ret[2];
  glfwGetFramebufferSize(get_window(M, self), &w, &h);
  ret[0] = mrb_fixnum_value(w);
  ret[1] = mrb_fixnum_value(h);
  return mrb_ary_new_from_values(M, 2, ret);
}

static mrb_value
window_frame_size(mrb_state *M, mrb_value self)
{
  int l, t, r, b;
  mrb_value ret[4];
  glfwGetWindowFrameSize(get_window(M, self), &l, &t, &r, &b);
  ret[0] = mrb_fixnum_value(l);
  ret[1] = mrb_fixnum_value(t);
  ret[2] = mrb_fixnum_value(r);
  ret[3] = mrb_fixnum_value(b);
  return mrb_ary_new_from_values(M, 4, ret);
}

static mrb_value
window_iconify(mrb_state *M, mrb_value self)
{
  glfwIconifyWindow(get_window(M, self));
  return self;
}

static mrb_value
window_restore(mrb_state *M, mrb_value self)
{
  glfwRestoreWindow(get_window(M, self));
  return self;
}

static mrb_value
window_show(mrb_state *M, mrb_value self)
{
  glfwShowWindow(get_window(M, self));
  return self;
}

static mrb_value
window_hide(mrb_state *M, mrb_value self)
{
  glfwHideWindow(get_window(M, self));
  return self;
}

static mrb_value
window_get_monitor(mrb_state *mrb, mrb_value self)
{
  return mrb_glfw3_monitor_value(mrb, glfwGetWindowMonitor(get_window(mrb, self)));
}

static mrb_value
window_get_window_attrib(mrb_state *mrb, mrb_value self)
{
  mrb_int attrib;
  mrb_get_args(mrb, "i", &attrib);
  return mrb_fixnum_value(glfwGetWindowAttrib(get_window(mrb, self), attrib));
}

static mrb_value
window_get_input_mode(mrb_state *mrb, mrb_value self)
{
  mrb_int mode;
  mrb_get_args(mrb, "i", &mode);
  return mrb_fixnum_value(glfwGetInputMode(get_window(mrb, self), mode));
}

static mrb_value
window_set_input_mode(mrb_state *mrb, mrb_value self)
{
  mrb_int mode;
  mrb_int value;
  mrb_get_args(mrb, "ii", &mode, &value);
  glfwSetInputMode(get_window(mrb, self), mode, value);
  return self;
}

CALLBACK_SETUP_N2(glfwSetWindowPosCallback, pos, int, int);
CALLBACK_SETUP_N2(glfwSetWindowSizeCallback, size, int, int);
CALLBACK_SETUP_N0(glfwSetWindowCloseCallback, close);
CALLBACK_SETUP_N0(glfwSetWindowRefreshCallback, refresh);
CALLBACK_SETUP_N1(glfwSetWindowFocusCallback, focus, int);
CALLBACK_SETUP_N1(glfwSetWindowIconifyCallback, iconify, int);
CALLBACK_SETUP_N2(glfwSetFramebufferSizeCallback, framebuffer_size, int, int);
CALLBACK_SETUP_N4(glfwSetKeyCallback, key, int, int, int, int);
CALLBACK_SETUP_N1(glfwSetCharCallback, char, uint);
CALLBACK_SETUP_N2(glfwSetCharModsCallback, char_mods, uint, int);
CALLBACK_SETUP_N3(glfwSetMouseButtonCallback, mouse_button, int, int, int);
CALLBACK_SETUP_N2(glfwSetCursorPosCallback, cursor_pos, double, double);
CALLBACK_SETUP_N1(glfwSetCursorEnterCallback, cursor_enter, int);
CALLBACK_SETUP_N2(glfwSetScrollCallback, scroll, double, double);
CALLBACK_SETUP_N_ary(glfwSetDropCallback, drop, const char**, string);

void
mrb_glfw3_window_init(mrb_state* mrb, struct RClass *mod)
{
  cb_MRB = mrb;
  mrb_glfw3_window_class = mrb_define_class_under(mrb, mod, "Window", mrb->object_class);
  MRB_SET_INSTANCE_TT(mrb_glfw3_window_class, MRB_TT_DATA);
  mrb_define_method(mrb, mrb_glfw3_window_class, "initialize",        window_initialize,        MRB_ARGS_REQ(3) | MRB_ARGS_OPT(2));
  mrb_define_method(mrb, mrb_glfw3_window_class, "destroy",           window_destroy,           MRB_ARGS_NONE());
  mrb_define_method(mrb, mrb_glfw3_window_class, "should_close",      window_get_should_close,  MRB_ARGS_NONE());
  mrb_define_method(mrb, mrb_glfw3_window_class, "should_close?",     window_get_should_close,  MRB_ARGS_NONE());
  mrb_define_method(mrb, mrb_glfw3_window_class, "should_close=",     window_set_should_close,  MRB_ARGS_REQ(1));
  mrb_define_method(mrb, mrb_glfw3_window_class, "title=",            window_set_title,         MRB_ARGS_REQ(1));
  mrb_define_method(mrb, mrb_glfw3_window_class, "window_pos",        window_pos_get,           MRB_ARGS_NONE());
  mrb_define_method(mrb, mrb_glfw3_window_class, "window_pos=",       window_pos_set,           MRB_ARGS_REQ(1));
  mrb_define_method(mrb, mrb_glfw3_window_class, "cursor_pos",        window_cursor_pos_get,    MRB_ARGS_NONE());
  mrb_define_method(mrb, mrb_glfw3_window_class, "cursor_pos=",       window_cursor_pos_set,    MRB_ARGS_REQ(1));
  mrb_define_method(mrb, mrb_glfw3_window_class, "window_size",       window_size_get,          MRB_ARGS_NONE());
  mrb_define_method(mrb, mrb_glfw3_window_class, "window_size=",      window_size_set,          MRB_ARGS_REQ(1));
  mrb_define_method(mrb, mrb_glfw3_window_class, "framebuffer_size",  window_framebuffer_size,  MRB_ARGS_NONE());
  mrb_define_method(mrb, mrb_glfw3_window_class, "window_frame_size", window_frame_size,        MRB_ARGS_NONE());
  mrb_define_method(mrb, mrb_glfw3_window_class, "iconify",           window_iconify,           MRB_ARGS_NONE());
  mrb_define_method(mrb, mrb_glfw3_window_class, "restore",           window_restore,           MRB_ARGS_NONE());
  mrb_define_method(mrb, mrb_glfw3_window_class, "show",              window_show,              MRB_ARGS_NONE());
  mrb_define_method(mrb, mrb_glfw3_window_class, "hide",              window_hide,              MRB_ARGS_NONE());
  mrb_define_method(mrb, mrb_glfw3_window_class, "monitor",           window_get_monitor,       MRB_ARGS_NONE());
  mrb_define_method(mrb, mrb_glfw3_window_class, "clipboard",         window_get_clipboard,     MRB_ARGS_NONE());
  mrb_define_method(mrb, mrb_glfw3_window_class, "clipboard=",        window_set_clipboard,     MRB_ARGS_REQ(1));
  mrb_define_method(mrb, mrb_glfw3_window_class, "window_attrib",     window_get_window_attrib, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, mrb_glfw3_window_class, "make_current",      window_make_current,      MRB_ARGS_NONE());
  mrb_define_method(mrb, mrb_glfw3_window_class, "swap_buffers",      window_swap_buffers,      MRB_ARGS_NONE());
  mrb_define_method(mrb, mrb_glfw3_window_class, "get_input_mode",    window_get_input_mode,    MRB_ARGS_REQ(1));
  mrb_define_method(mrb, mrb_glfw3_window_class, "set_input_mode",    window_set_input_mode,    MRB_ARGS_REQ(2));

  /* Callbacks */
  mrb_define_method(mrb, mrb_glfw3_window_class, "set_pos_callback",              window_set_pos_callback,              MRB_ARGS_BLOCK());
  mrb_define_method(mrb, mrb_glfw3_window_class, "set_size_callback",             window_set_size_callback,             MRB_ARGS_BLOCK());
  mrb_define_method(mrb, mrb_glfw3_window_class, "set_close_callback",            window_set_close_callback,            MRB_ARGS_BLOCK());
  mrb_define_method(mrb, mrb_glfw3_window_class, "set_refresh_callback",          window_set_refresh_callback,          MRB_ARGS_BLOCK());
  mrb_define_method(mrb, mrb_glfw3_window_class, "set_focus_callback",            window_set_focus_callback,            MRB_ARGS_BLOCK());
  mrb_define_method(mrb, mrb_glfw3_window_class, "set_iconify_callback",          window_set_iconify_callback,          MRB_ARGS_BLOCK());
  mrb_define_method(mrb, mrb_glfw3_window_class, "set_framebuffer_size_callback", window_set_framebuffer_size_callback, MRB_ARGS_BLOCK());
  mrb_define_method(mrb, mrb_glfw3_window_class, "set_key_callback",              window_set_key_callback,              MRB_ARGS_BLOCK());
  mrb_define_method(mrb, mrb_glfw3_window_class, "set_char_callback",             window_set_char_callback,             MRB_ARGS_BLOCK());
  mrb_define_method(mrb, mrb_glfw3_window_class, "set_char_mods_callback",        window_set_char_mods_callback,        MRB_ARGS_BLOCK());
  mrb_define_method(mrb, mrb_glfw3_window_class, "set_mouse_button_callback",     window_set_mouse_button_callback,     MRB_ARGS_BLOCK());
  mrb_define_method(mrb, mrb_glfw3_window_class, "set_cursor_pos_callback",       window_set_cursor_pos_callback,       MRB_ARGS_BLOCK());
  mrb_define_method(mrb, mrb_glfw3_window_class, "set_cursor_enter_callback",     window_set_cursor_enter_callback,     MRB_ARGS_BLOCK());
  mrb_define_method(mrb, mrb_glfw3_window_class, "set_scroll_callback",           window_set_scroll_callback,           MRB_ARGS_BLOCK());
  mrb_define_method(mrb, mrb_glfw3_window_class, "set_drop_callback",             window_set_drop_callback,             MRB_ARGS_BLOCK());
}
