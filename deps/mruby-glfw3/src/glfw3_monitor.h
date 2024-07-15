#ifndef MRB_GLFW3_MONITOR_H
#define MRB_GLFW3_MONITOR_H

#include <mruby.h>
#include <mruby/data.h>
#include <mruby/class.h>

#include <GLFW/glfw3.h>

extern const struct mrb_data_type mrb_glfw3_monitor_type;
void mrb_glfw3_monitor_init(mrb_state *mrb, struct RClass *mod);
mrb_value mrb_glfw3_monitor_value(mrb_state *mrb, GLFWmonitor *mon);

#endif
