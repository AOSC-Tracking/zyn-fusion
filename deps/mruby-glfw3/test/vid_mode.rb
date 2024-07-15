=begin
$mrbtest_verbose = true

GLFW.init

assert('GLFW::VidMode type') do
  assert_kind_of(Class, GLFW::VidMode)
end

monitor = GLFW.primary_monitor
if monitor
  video_mode = monitor.video_mode

  assert('GLFW::VidMode#width') do
    assert_kind_of(Integer, video_mode.width)
  end

  assert('GLFW::VidMode#height') do
    assert_kind_of(Integer, video_mode.height)
  end

  assert('GLFW::VidMode#red_bits') do
    assert_kind_of(Integer, video_mode.red_bits)
  end

  assert('GLFW::VidMode#green_bits') do
    assert_kind_of(Integer, video_mode.green_bits)
  end

  assert('GLFW::VidMode#blue_bits') do
    assert_kind_of(Integer, video_mode.blue_bits)
  end

  assert('GLFW::VidMode#refresh_rate') do
    assert_kind_of(Integer, video_mode.refresh_rate)
  end
end
=end
