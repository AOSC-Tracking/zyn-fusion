win = GLFW::Window.new 600, 400, "Triangle"
win.make_current

include GL2

def load_shader type, src
  shader = glCreateShader type
  glShaderSource shader, 1, [src], [src.length]
  glCompileShader shader
  compile_result = glGetShaderiv shader, GL_COMPILE_STATUS
  if compile_result == GL_FALSE
    info_len = glGetShaderiv shader, GL_INFO_LOG_LENGTH

    p glGetShaderInfoLog shader, info_len
    raise RuntimeError.new glGetShaderInfoLog(shader, info_len) if info_len > 0
    glDeleteShader shader
    return nil
  end
  shader
end

def link_program prog
  glLinkProgram prog
  linked = glGetProgramiv prog, GL_LINK_STATUS
  if linked == GL_FALSE
    info_len = glGetProgramiv prog, GL_INFO_LOG_LENGTH
    raise glGetProgramInfoLog(prog, info_len)[1] if info_len > 0
    glDeleteProgram prog
  end
end

def check_error
  err = glGetError
  if err != GL_NO_ERROR
    case err
    when GL_INVALID_ENUM; err = 'invalid enum'
    when GL_INVALID_VALUE; err = 'invalid value'
    when GL_INVALID_OPERATION; err = 'invalid operation'
    when GL_INVALID_FRAMEBUFFER_OPERATION; 'invalid framebuffer operation'
    when GL_OUT_OF_MEMORY; err = 'out of memory'
    else; err = 'unknown GL error'
    end
    raise err
  end
end

vtex = load_shader GL_VERTEX_SHADER, <<EOS
attribute vec4 a_position;
attribute vec4 a_color;

varying vec4 v_color;

void main() {
  v_color = a_color;
  gl_Position = a_position;
}
EOS

frag = load_shader GL_FRAGMENT_SHADER, <<EOS
varying vec4 v_color;

void main() {
  gl_FragColor = v_color;
}
EOS

prog = glCreateProgram
raise 'failed to create program' if prog == 0
glAttachShader prog, vtex
glAttachShader prog, frag
link_program prog
pos_idx = glGetAttribLocation prog, 'a_position'
col_idx = glGetAttribLocation prog, 'a_color'
raise if pos_idx == -1 or col_idx == -1
glEnableVertexAttribArray pos_idx
glEnableVertexAttribArray col_idx

pos_buffer, col_buffer = glGenBuffers 2

glBindBuffer GL_ARRAY_BUFFER, pos_buffer
data = [
  0.0, 0.8, 0.0, 1.0,
  -0.8, -0.8, 0.0, 1.0,
  0.8, -0.8, 0.0, 1.0].pack 'f*'
glBufferData GL_ARRAY_BUFFER, data.length, data, GL_STATIC_DRAW

glBindBuffer GL_ARRAY_BUFFER, col_buffer
data = [
  1.0, 0.0, 0.0, 1.0,
  0.0, 1.0, 0.0, 1.0,
  0.0, 0.0, 1.0, 1.0].pack 'f*'
glBufferData GL_ARRAY_BUFFER, data.length, data, GL_STATIC_DRAW

glClearColor 1.0, 1.0, 1.0, 1.0
until win.should_close?
  w, h = win.window_size
  glViewport 0, 0, w, h
  glClear GL_COLOR_BUFFER_BIT

  glEnable GL_BLEND
  glBlendFunc GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA
  glUseProgram prog
  glBindBuffer GL_ARRAY_BUFFER, pos_buffer
  glVertexAttribPointer pos_idx, 4, GL_FLOAT, false, 0, nil
  glBindBuffer GL_ARRAY_BUFFER, col_buffer
  glVertexAttribPointer col_idx, 4, GL_FLOAT, false, 0, nil
  glDrawArrays GL_TRIANGLES, 0, 3

  check_error

  win.swap_buffers
  GLFW.wait_events
end
