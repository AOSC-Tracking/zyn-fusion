module GLEW
  # Initializes GLEW
  # @return [self]
  def self.init
  end

  # Checks if the given extension is supported.
  #
  # @param [String] name
  # @return [Boolean]
  def self.supported?(name)
  end

  # Checks if the given extension is supported.
  #
  # @param [String] name
  # @return [Boolean]
  def self.get_extension(name)
  end

  # Returns the error corresponding to the error code
  #
  # @param [Integer] error  the error enum
  # @return [String, nil] returns the string, or nil if the error was invalid
  def self.get_error_string(error)
  end

  #
  # @param [Integer] name  the enum
  # @return [String, nil] returns the string, or nil if the name was invalid
  def self.get_string(name)
  end
end
