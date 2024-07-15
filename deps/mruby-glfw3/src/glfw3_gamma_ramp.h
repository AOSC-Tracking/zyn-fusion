#ifndef MRB_GLFW3_GAMMA_RAMP_H
#define MRB_GLFW3_GAMMA_RAMP_H

#include <mruby.h>
#include <mruby/data.h>
#include <mruby/class.h>

#include <GLFW/glfw3.h>

extern const struct mrb_data_type mrb_glfw3_gamma_ramp_type;
void mrb_glfw3_gamma_ramp_init(mrb_state *mrb, struct RClass *mod);
mrb_value mrb_glfw3_gamma_ramp_value(mrb_state *mrb, const GLFWgammaramp *gammaramp);

#endif
