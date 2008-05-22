module Telegraph
  module Core
    class Router
      def initialize
        @routes = []
      end
      
      def connect(name, &block)
        @routes << {:controller=>name, :block=>block}
      end
      
      def default(name)
        @routes << {:controller=>name, :block=>nil}
      end
      
      def dispatch!(event_name, params)
        controller_name = @routes.detect{|r| 
          r[:block].nil? || r[:block].call(params)}[:controller]
        controller = controller_from_name(controller_name)
        controller.params = params
        controller.send(event_name.to_sym)
      end
      
      def used_events
        @routes.map{|r| controller_from_name(r[:controller]).methods}.flatten - Telegraph::Core::VoiceEventsBase.new.methods
      end
      
      private
      def controller_from_name(name)
        "#{name}_voice_events".camelize.constantize.new
      end
    end
  end
end