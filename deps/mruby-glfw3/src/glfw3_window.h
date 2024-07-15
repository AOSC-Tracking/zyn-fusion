#ifndef MRB_GLFW3_WINDOW_H
#define MRB_GLFW3_WINDOW_H

#include <mruby.h>
#include <mruby/data.h>
#include <mruby/class.h>

extern const struct mrb_data_type mrb_glfw3_window_type;
void mrb_glfw3_window_init(mrb_state *mrb, struct RClass *mod);

#endif
