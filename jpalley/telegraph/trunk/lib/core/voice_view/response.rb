module Telegraph
  module VoiceView
    class Response < ActionController::AbstractResponse
      attr_accessor :interface
      
      def initialize(interface)
        @interface = interface
        @commands = []
        super()
      end
      
      def out(output)
      end
      
      def commands
        @interface.commands
      end
    end
  end
end