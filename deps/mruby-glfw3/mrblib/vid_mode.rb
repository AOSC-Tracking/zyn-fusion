module GLFW
  class VidMode
    def inspect
      str = super.dup
      str.slice(0, str.size - 1) + " width=#{width} height=#{height} red_bits=#{red_bits} green_bits=#{green_bits} blue_bits=#{blue_bits} refresh_rate=#{refresh_rate}>"
    end
  end
end
