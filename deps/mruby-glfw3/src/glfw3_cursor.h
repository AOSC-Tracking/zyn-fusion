#ifndef MRB_GLFW3_CURSOR_H
#define MRB_GLFW3_CURSOR_H

#include <mruby.h>
#include <mruby/data.h>
#include <mruby/class.h>

#include <GLFW/glfw3.h>

extern const struct mrb_data_type mrb_glfw3_cursor_type;
void mrb_glfw3_cursor_init(mrb_state *mrb, struct RClass *mod);
mrb_value mrb_glfw3_cursor_value(mrb_state *mrb, GLFWcursor *cursor);

#endif
