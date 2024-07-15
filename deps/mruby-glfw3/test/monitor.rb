=begin
$mrbtest_verbose = true

GLFW.init

assert('GLFW::Monitor type') do
  assert_kind_of(Class, GLFW::Monitor)
end

assert('GLFW.monitors') do
  ary = GLFW.monitors
  assert_kind_of(Array, ary)
  ary.all? { |a| assert_kind_of(GLFW::Monitor, a) }
end

assert('GLFW.primary_monitor') do
  assert_kind_of(GLFW::Monitor, GLFW.primary_monitor)
  true
end

assert('GLFW.set_monitor_callback') do
  GLFW.set_monitor_callback do
  end
  true
end

monitor = GLFW.primary_monitor

if monitor
  assert('GLFW::Monitor#position') do
    result = monitor.position
    assert_kind_of(Array, result)
  end

  assert('GLFW::Monitor#physical_size') do
    result = monitor.physical_size
    assert_kind_of(Array, result)
  end

  assert('GLFW::Monitor#name') do
    result = monitor.name
    assert_kind_of(String, result)
  end

  assert('GLFW::Monitor#video_modes') do
    result = monitor.video_modes
    assert_kind_of(Array, result)
    result.all? { |a| assert_kind_of(GLFW::VidMode, a) }
  end

  assert('GLFW::Monitor#video_mode') do
    result = monitor.video_mode
    assert_kind_of(GLFW::VidMode, result)
  end

  assert('GLFW::Monitor#gamma=') do
    # DANGER
    true
  end

  assert('GLFW::Monitor#gamma_ramp') do
    result = monitor.gamma_ramp
    assert_kind_of(GLFW::GammaRamp, result)
  end

  assert('GLFW::Monitor#gamma_ramp=') do
    # DANGER
    true
  end
end
=end
