#include <mruby.h>
#include <mruby/data.h>
#include <mruby/array.h>
#include <mruby/numeric.h>

#include <GLFW/glfw3.h>

#include "glfw3_cursor.h"
#include "glfw3_image.h"
#include "glfw3_window.h"

static struct RClass *mrb_glfw3_cursor_class;

void
mrb_glfw_cursor_free(mrb_state *mrb, void *ptr)
{
  GLFWcursor *cursor = ptr;
  if (cursor) {
    glfwDestroyCursor(cursor);
  }
}

const struct mrb_data_type mrb_glfw3_cursor_type = { "GLFWcursor", mrb_glfw_cursor_free };

mrb_value
mrb_glfw3_cursor_value(mrb_state *mrb, GLFWcursor *cursor)
{
  mrb_value result;
  result = mrb_obj_new(mrb, mrb_glfw3_cursor_class, 0, NULL);
  DATA_PTR(result) = cursor;
  DATA_TYPE(result) = &mrb_glfw3_cursor_type;
  return result;
}

static mrb_value
cursor_s_create(mrb_state *mrb, mrb_value klass)
{
  mrb_int xhot;
  mrb_int yhot;
  GLFWimage *image;
  mrb_get_args(mrb, "dii", &image, &mrb_glfw3_image_type, &xhot, &yhot);
  return mrb_glfw3_cursor_value(mrb, glfwCreateCursor(image, xhot, yhot));
}

static mrb_value
cursor_s_create_standard(mrb_state *mrb, mrb_value klass)
{
  mrb_int shape;
  mrb_get_args(mrb, "i", &shape);
  return mrb_glfw3_cursor_value(mrb, glfwCreateStandardCursor(shape));
}

static mrb_value
cursor_s_set(mrb_state *mrb, mrb_value klass)
{
  GLFWwindow *window;
  GLFWcursor *cursor;
  mrb_get_args(mrb, "dd", &window, &mrb_glfw3_window_type, &cursor, &mrb_glfw3_cursor_type);
  glfwSetCursor(window, cursor);
  return klass;
}

void
mrb_glfw3_cursor_init(mrb_state *mrb, struct RClass *mod)
{
  mrb_glfw3_cursor_class = mrb_define_class_under(mrb, mod, "Cursor", mrb->object_class);
  MRB_SET_INSTANCE_TT(mrb_glfw3_cursor_class, MRB_TT_DATA);

  mrb_define_class_method(mrb, mrb_glfw3_cursor_class, "create",          cursor_s_create,          MRB_ARGS_REQ(3));
  mrb_define_class_method(mrb, mrb_glfw3_cursor_class, "create_standard", cursor_s_create_standard, MRB_ARGS_REQ(1));
  mrb_define_class_method(mrb, mrb_glfw3_cursor_class, "set",             cursor_s_set,             MRB_ARGS_REQ(1));
}
