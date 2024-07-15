module GLFW
  def self.joystick_present?(id)
    joystick_present(id) == GL2::GL_TRUE
  end
end
