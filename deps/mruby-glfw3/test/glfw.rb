=begin
$mrbtest_verbose = true

GLFW.init

assert('GLFW type') do
  assert_kind_of(Module, GLFW)
end

assert('GLFW.init') do
  GLFW.init
end

assert('GLFW.current_context') do
  assert_equal(nil, GLFW.current_context)

  window = GLFW::Window.new(320, 240, 'Context Test')
  window.make_current

  GLFW.cache_size

  assert_equal(window, GLFW.current_context)

  until window.should_close?
    GL2.glClear(GL2::GL_COLOR_BUFFER_BIT)
    window.swap_buffers
    GLFW.poll_events
  end

  window.destroy

  true
end

assert('GLFW.swap_interval=') do
  GLFW.swap_interval = 60
  true
end

assert('GLFW.extension_supported?') do
  GLFW.extension_supported?("SOME EXTENSION")
  true
end

assert('GLFW.proc_address') do
  # DANGER
  GLFW.proc_address 'mrbtest'
  true
end

assert('GLFW.version') do
  GLFW.version
  true
end

assert('GLFW.version_string') do
  GLFW.version_string
  true
end

assert('GLFW.time') do
  assert_kind_of(Float, GLFW.time)
end

assert('GLFW.time=') do
  GLFW.time = 0
  true
end

assert('GLFW.poll_events') do
  window = GLFW::Window.new(320, 240, 'Poll Events test')
  window.make_current

  until window.should_close?
    window.swap_buffers
    GLFW.poll_events
  end

  window.destroy
  true
end

assert('GLFW.wait_events') do
  window = GLFW::Window.new(320, 240, 'Wait Events test')
  window.make_current

  window.swap_buffers
  window.show

  until window.should_close?
    GL2.glClear(GL2::GL_COLOR_BUFFER_BIT)
    window.swap_buffers
    # may cause hangup
    GLFW.wait_events
    GLFW.poll_events
  end

  window.destroy
  true
end

assert('GLFW.default_window_hints') do
  GLFW.default_window_hints
  window = GLFW::Window.new(320, 240, 'default window hints test')
  window.make_current

  until window.should_close?
    window.swap_buffers
    GLFW.poll_events
  end
  true
end

assert('GLFW.window_hint') do
  GLFW::Window.default_hints
  GLFW::Window.hint(GLFW::RESIZABLE, GL2::GL_FALSE);
  GLFW::Window.hint(GLFW::CONTEXT_VERSION_MAJOR, 2);
  GLFW::Window.hint(GLFW::CONTEXT_VERSION_MINOR, 1);
  window = GLFW::Window.new(320, 240, 'Window hint test')
  window.make_current

  until window.should_close?
    window.swap_buffers
    GLFW.poll_events
  end

  window.destroy

  true
end

=end
