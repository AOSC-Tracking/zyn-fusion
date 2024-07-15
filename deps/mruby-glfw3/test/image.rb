assert('GLFW::Image type') do
  assert_kind_of(Class, GLFW::Image)
end

assert('GLFW::Image#initialize') do
  img = GLFW::Image.new(32, 32)
  assert_kind_of(GLFW::Image, img)
end

assert('GLFW::Image#[]') do
  img = GLFW::Image.new(32, 32)
  assert_kind_of(Array, img[0, 0])
  assert_equal([0, 0, 0, 0], img[0, 0])
  img[0, 0] = [0, 0, 0, 255]
  assert_equal([0, 0, 0, 255], img[0, 0])
end
