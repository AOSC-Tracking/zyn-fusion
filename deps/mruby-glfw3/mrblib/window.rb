module GLFW
  class Window
    alias :position :window_pos
    alias :position= :window_pos=
    alias :attrib :window_attrib

    def input_mode(*args)
      if args.size == 1
        get_input_mode(args.first)
      elsif args.size == 2
        set_input_mode(*args)
      else
        raise ArgumentError, 'expected 1 or 2'
      end
    end

    alias :should_close_raw_set :should_close=
    def should_close=(value)
      if value == true || value == false
        should_close_raw_set(value ? GL2::GL_TRUE : GL2::GL_FALSE)
      else
        should_close_raw_set(value)
      end
    end

    def set_cursor(cursor)
      GLFW::Cursor.set(self, cursor)
    end

    def self.default_hints
      GLFW.default_window_hints
    end

    def self.hint(*args)
      GLFW.window_hint(*args)
    end
  end
end
