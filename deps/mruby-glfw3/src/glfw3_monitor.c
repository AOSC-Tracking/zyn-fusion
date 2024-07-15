#include <stdbool.h>

#include <mruby.h>
#include <mruby/array.h>
#include <mruby/class.h>
#include <mruby/data.h>
#include <mruby/hash.h>
#include <mruby/variable.h>
#include <mruby/string.h>

#include <GLFW/glfw3.h>

#include "glfw3_monitor.h"
#include "glfw3_vid_mode.h"
#include "glfw3_gamma_ramp.h"

static struct RClass *mrb_glfw3_monitor_class;
const struct mrb_data_type mrb_glfw3_monitor_type = { "GLFWmonitor", NULL };

static inline GLFWmonitor*
get_monitor(mrb_state *mrb, mrb_value self)
{
  return (GLFWmonitor*)mrb_data_get_ptr(mrb, self, &mrb_glfw3_monitor_type);
}

mrb_value
mrb_glfw3_monitor_value(mrb_state *mrb, GLFWmonitor *mon)
{
  mrb_value monitor = mrb_obj_new(mrb, mrb_glfw3_monitor_class, 0, NULL);
  DATA_PTR(monitor) = mon;
  DATA_TYPE(monitor) = &mrb_glfw3_monitor_type;
  return monitor;
}

static mrb_value
glfw_s_monitors(mrb_state *mrb, mrb_value klass)
{
  mrb_value result;
  int count;
  int i;
  GLFWmonitor **monitors;
  result = mrb_ary_new(mrb);
  monitors = glfwGetMonitors(&count);
  for (i = 0; i < count; ++i) {
    mrb_ary_push(mrb, result, mrb_glfw3_monitor_value(mrb, monitors[i]));
  }
  return result;
}

static mrb_value
glfw_s_primary_monitor(mrb_state *mrb, mrb_value klass)
{
  GLFWmonitor *monitor = glfwGetPrimaryMonitor();
  return mrb_glfw3_monitor_value(mrb, monitor);
}

static mrb_value
glfw_s_set_monitor_callback(mrb_state *mrb, mrb_value klass)
{
  return klass;
}

static mrb_value
monitor_position(mrb_state *mrb, mrb_value self)
{
  mrb_value vals[2];
  int xpos;
  int ypos;
  glfwGetMonitorPos(get_monitor(mrb, self), &xpos, &ypos);
  vals[0] = mrb_fixnum_value(xpos);
  vals[1] = mrb_fixnum_value(ypos);
  return mrb_ary_new_from_values(mrb, 2, vals);
}

static mrb_value
monitor_physical_size(mrb_state *mrb, mrb_value self)
{
  mrb_value vals[2];
  int widthMM;
  int heightMM;
  glfwGetMonitorPhysicalSize(get_monitor(mrb, self), &widthMM, &heightMM);
  vals[0] = mrb_fixnum_value(widthMM);
  vals[1] = mrb_fixnum_value(heightMM);
  return mrb_ary_new_from_values(mrb, 2, vals);
}

static mrb_value
monitor_name(mrb_state *mrb, mrb_value self)
{
  const char *name = glfwGetMonitorName(get_monitor(mrb, self));
  return mrb_str_new_cstr(mrb, name);
}

static mrb_value
monitor_vid_modes(mrb_state *mrb, mrb_value self)
{
  int count;
  int i;
  mrb_value result = mrb_ary_new(mrb);
  const GLFWvidmode *vid_modes = glfwGetVideoModes(get_monitor(mrb, self), &count);
  for (i = 0; i < count; ++i) {
    mrb_ary_push(mrb, result, mrb_glfw3_vid_mode_value(mrb, vid_modes[i]));
  }
  return result;
}

static mrb_value
monitor_vid_mode(mrb_state *mrb, mrb_value self)
{
  return mrb_glfw3_vid_mode_value(mrb, *glfwGetVideoMode(get_monitor(mrb, self)));
}

static mrb_value
monitor_set_gamma(mrb_state *mrb, mrb_value self)
{
  mrb_float gamma;
  mrb_get_args(mrb, "f", &gamma);
  glfwSetGamma(get_monitor(mrb, self), gamma);
  return mrb_nil_value();
}

static mrb_value
monitor_get_gamma_ramp(mrb_state *mrb, mrb_value self)
{
  const GLFWgammaramp *gammaramp = glfwGetGammaRamp(get_monitor(mrb, self));
  return mrb_glfw3_gamma_ramp_value(mrb, gammaramp);
}

static mrb_value
monitor_set_gamma_ramp(mrb_state *mrb, mrb_value self)
{
  GLFWgammaramp *gammaramp;
  mrb_get_args(mrb, "d", &gammaramp, &mrb_glfw3_gamma_ramp_type);
  glfwSetGammaRamp(get_monitor(mrb, self), gammaramp);
  return mrb_nil_value();
}

void
mrb_glfw3_monitor_init(mrb_state* mrb, struct RClass *mod)
{
  mrb_define_class_method(mrb, mod, "monitors",             glfw_s_monitors,             MRB_ARGS_NONE());
  mrb_define_class_method(mrb, mod, "primary_monitor",      glfw_s_primary_monitor,      MRB_ARGS_NONE());
  mrb_define_class_method(mrb, mod, "set_monitor_callback", glfw_s_set_monitor_callback, MRB_ARGS_BLOCK());

  mrb_glfw3_monitor_class = mrb_define_class_under(mrb, mod, "Monitor", mrb->object_class);
  MRB_SET_INSTANCE_TT(mrb_glfw3_monitor_class, MRB_TT_DATA);
  mrb_define_method(mrb, mrb_glfw3_monitor_class, "position",      monitor_position,       MRB_ARGS_NONE());
  mrb_define_method(mrb, mrb_glfw3_monitor_class, "physical_size", monitor_physical_size,  MRB_ARGS_NONE());
  mrb_define_method(mrb, mrb_glfw3_monitor_class, "name",          monitor_name,           MRB_ARGS_NONE());
  mrb_define_method(mrb, mrb_glfw3_monitor_class, "vid_modes",   monitor_vid_modes,    MRB_ARGS_NONE());
  mrb_define_method(mrb, mrb_glfw3_monitor_class, "vid_mode",    monitor_vid_mode,     MRB_ARGS_NONE());
  mrb_define_method(mrb, mrb_glfw3_monitor_class, "gamma=",        monitor_set_gamma,      MRB_ARGS_REQ(1));
  mrb_define_method(mrb, mrb_glfw3_monitor_class, "gamma_ramp",    monitor_get_gamma_ramp, MRB_ARGS_NONE());
  mrb_define_method(mrb, mrb_glfw3_monitor_class, "gamma_ramp=",   monitor_set_gamma_ramp, MRB_ARGS_REQ(1));
}
