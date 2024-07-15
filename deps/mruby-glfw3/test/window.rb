=begin
$mrbtest_verbose = true

GLFW.init

assert('GLFW::Window type') do
  assert_kind_of(Class, GLFW::Window)
end

assert('GLFW::Window#initialize') do
  window = GLFW::Window.new(320, 240, 'Window (initialize) test')
  window.make_current

  until window.should_close?
    GL2.glClear(GL2::GL_COLOR_BUFFER_BIT)
    window.swap_buffers
    GLFW.poll_events
  end

  window.destroy
  true
end

assert('GLFW::Window#clipboard') do
  window = GLFW::Window.new(320, 240, 'Window clipboard test')
  window.make_current
  assert_kind_of(String, window.clipboard)

  until window.should_close?
    GL2.glClear(GL2::GL_COLOR_BUFFER_BIT)
    window.swap_buffers
    GLFW.poll_events
  end

  window.should_close = true
  window.destroy

  true
end

assert('GLFW::Window#swap_buffers') do
  true
end

assert('GLFW::Window#should_close') do
  true
end

assert('GLFW::Window#should_close?') do
  true
end

assert('GLFW::Window#should_close=') do
  true
end

assert('GLFW::Window#window_size') do
  true
end

assert('GLFW::Window#window_size=') do
  true
end

assert('GLFW::Window#window_pos') do
  true
end

assert('GLFW::Window#window_pos=') do
  true
end

assert('GLFW::Window#framebuffer_size') do
  true
end

assert('GLFW::Window#window_frame_size') do
  true
end

assert('GLFW::Window#iconify') do
  true
end

assert('GLFW::Window#restore') do
  true
end

assert('GLFW::Window#show') do
  true
end

assert('GLFW::Window#hide') do
  true
end

=end
