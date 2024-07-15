module GLFW
  class Joystick
    # @return [Integer]
    attr_reader :handle

    def initialize(handle)
      @handle = handle
    end

    # @return [Integer]
    def present
      GLFW.joystick_present(@handle)
    end

    # @return [Boolean]
    def present?
      GLFW.joystick_present?(@handle)
    end

    def axes
      GLFW.joystick_axes(@handle)
    end

    def buttons
      GLFW.joystick_buttons(@handle)
    end
  end
end
