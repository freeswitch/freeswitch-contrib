module Telegraph
  module VoiceView
    class StdOutEmulator
        def write(str)
        end
    end
    class Dispatcher
      attr_accessor :request
      attr_accessor :response
      attr_accessor :params
      
      require 'action_controller/dispatcher'
      
      def dispatch(interface)
         ENV['REQUEST_METHOD'] = "post"
 
         @cgi=CGI.new
         @request = Telegraph::VoiceView::Request.new(interface,@cgi,nil,nil,ActionController::CgiRequest::DEFAULT_SESSION_OPTIONS)  

         @response = Telegraph::VoiceView::Response.new(interface)
        
         prepare_application
            
         ActionController::Routing::Routes.recognize(@request)

         # bit of a hack.  Need to setup next_action/next_controller
         
         @request.next_action = @request.parameters['action']
         @request.next_controller = @request.parameters['controller'].camelize + 'Controller'
         
                    
          #Loops until we are done executing all the actions for this call
          while @request.next_action !=nil do
            path_params={:action=>@request.next_action, :controller=>@request.next_controller}
            @request.path_parameters = path_params
            @request.next_action=nil
            ActionController::Dispatcher.new(StdOutEmulator.new, @request, @response).dispatch
            #Check for errors
            if @response.headers['Status'] == "500 Internal Server Error" or @response.headers['Status'] == "406 Not Acceptable"
              @request.interface.hangup
              break
            end

          end
          
          return @response.commands
          
      end
      
      def prepare_application
          ActionController::Routing::Routes.reload if Dependencies.load?
          require_dependency('application.rb') unless Object.const_defined?(:ApplicationController)
          ActiveRecord::Base.verify_active_connections!
      end
    end
  end
end
        