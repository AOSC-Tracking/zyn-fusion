#include <mruby.h>
#include <mruby/class.h>
#include <mruby/string.h>
#include <GL/glew.h>

static mrb_value
init(mrb_state *mrb, mrb_value klass)
{
  if (glewInit() != GLEW_OK) {
    mrb_raise(mrb, E_RUNTIME_ERROR, "GLEW has failed to initialize.");
  }
  return klass;
}

static mrb_value
set_experimental(mrb_state *mrb, mrb_value klass)
{
  mrb_bool expm;
  mrb_get_args(mrb, "b", &expm);
  glewExperimental = expm;
  return klass;
}

static mrb_value
get_experimental(mrb_state *mrb, mrb_value klass)
{
  return mrb_bool_value(glewExperimental);
}

static mrb_value
is_supported(mrb_state *mrb, mrb_value klass)
{
  char *name;
  mrb_get_args(mrb, "z", &name);
  return mrb_bool_value(glewIsSupported(name));
}

static mrb_value
get_extension(mrb_state *mrb, mrb_value klass)
{
  char *name;
  mrb_get_args(mrb, "z", &name);
  return mrb_bool_value(glewGetExtension(name));
}

static mrb_value
get_error_string(mrb_state *mrb, mrb_value klass)
{
  mrb_int error;
  char *str;
  mrb_get_args(mrb, "i", &error);
  str = (char*)glewGetErrorString((GLenum)error);
  if (str) {
    return mrb_str_new_cstr(mrb, str);
  } else {
    return mrb_nil_value();
  }
}

static mrb_value
get_string(mrb_state *mrb, mrb_value klass)
{
  mrb_int name;
  char *str;
  mrb_get_args(mrb, "i", &name);
  str = (char*)glewGetString((GLenum)name);
  if (str) {
    return mrb_str_new_cstr(mrb, str);
  } else {
    return mrb_nil_value();
  }
}

void
mrb_mruby_glew_gem_init(mrb_state* mrb)
{
  struct RClass *glew_mod = mrb_define_module(mrb, "GLEW");
  mrb_define_class_method(mrb, glew_mod, "init",             init,             MRB_ARGS_NONE());
  mrb_define_class_method(mrb, glew_mod, "experimental=",    set_experimental, MRB_ARGS_REQ(1));
  mrb_define_class_method(mrb, glew_mod, "experimental",     get_experimental, MRB_ARGS_NONE());
  mrb_define_class_method(mrb, glew_mod, "supported?",       is_supported,     MRB_ARGS_REQ(1));
  mrb_define_class_method(mrb, glew_mod, "get_extension",    get_extension,    MRB_ARGS_REQ(1));
  mrb_define_class_method(mrb, glew_mod, "get_error_string", get_error_string, MRB_ARGS_REQ(1));
  mrb_define_class_method(mrb, glew_mod, "get_string",       get_string,       MRB_ARGS_REQ(1));
}

void
mrb_mruby_glew_gem_final(mrb_state* mrb)
{
}
