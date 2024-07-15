module GLFW
  class Monitor
    def inspect
      str = super.dup
      str.slice(0, str.size - 1) + " position=#{position} physical_size=#{physical_size}>"
    end

    def self.list
      GLFW.monitors
    end

    def self.primary
      GLFW.primary_monitor
    end

    def self.set_callback(&block)
      GLFW.set_monitor_callback(&block)
    end
  end
end
