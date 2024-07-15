module GLFW
  class Image
    def inspect
      str = super.dup
      str.slice(0, str.size - 1) + " pixelsize=#{pixelsize} memsize=#{memsize} width=#{width} height=#{height}>"
    end

    alias :clear_ary :clear
    def clear(*args)
      if args.size == 4
        clear_ary(args)
      else
        clear_ary(*args)
      end
    end
  end
end
