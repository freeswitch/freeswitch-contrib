# Copyright (c) 2007 Jonathan Palley, Idapted Inc.
# All rights reserved

module Telegraph
  module VoiceView
    class Request < ActionController::AbstractRequest
      attr_accessor :session_options, :path, :session, :env
      attr_accessor :host, :interface, :next_action, :next_controller, :path
    
    #
      def initialize(interface, cgi, query_parameters ={}, request_parameters = {}, session_opts = ActionController::CgiRequest::DEFAULT_SESSION_OPTIONS)
        @interface = interface      
        @query_parameters   = query_parameters 
        @session_options            = session_opts
        @redirect = false
        @env = {}
    
        #these allow us to do redirects (or psuedo form submits)to other actions since we can't do a HTTP 302
        @next_action             = nil
        @next_controller         = nil
        @redirect_parameters     = nil
    
    
        @cgi = cgi
        @host                    = "voice_view"
        @request_uri             = "/"
        @env['REQUEST_METHOD']   = "POST"

        @path= @interface.url
        @session_options['session_id'] = @interface.params['session_id']
    
        
        super()
      end
      def sound?
        true
      end
      def accepts
        [Mime::Type.lookup_by_extension('voice')]
      end
      
 

    #copies parameters from the agi request (in the call_connection object)
    #if this is a redirect we add in the redict parameters
    def request_parameters
      @interface.params.dup
    end
    
    def query_parameters
      @redirect_parameters || {}
    end
    
    def remote_ip
      return '123.123.123.123'
    end
   
    

    def create_redirect(args)
    
      @next_action=args.delete(:action).to_s
      @next_controller = args.delete(:controller).to_s.camelize + 'Controller' unless args[:controller].nil?
      
      @redirect_parameters = args
      parameters
    end

     def remote_addr=(addr)
      @env['REMOTE_ADDR'] = addr
    end

    def remote_addr
      @env['REMOTE_ADDR']
    end

    def request_uri
      @request_uri || super()
    end

    def path
      @path || super()
    end
   

    def session=(session)
      @session = session
      @session.update
    end
    
    def session  
      unless @session
         if @session_options == false
            @session = Hash.new
        else
          stale_session_check! do
            
            if session_options_with_string_keys['new_session'] == true
              @session = new_session
            else
              @session = CGI::Session.new(@cgi, session_options_with_string_keys)
            end
            session['__valid_session']
          end
        end
      end
      @session
    end

    def reset_session
      @session.delete if CGI::Session === @session
      @session = new_session
    end

    def method_missing(method_id, *arguments)
      @cgi.send(method_id, *arguments) rescue super
    end
    
    def logger=(logger)
      @logger=logger
    end
    
    def logger
      @logger
    end
    
    private
      # Delete an old session if it exists then create a new one.
      def new_session
        if @session_options == false
          Hash.new
        else
          CGI::Session.new(@cgi, session_options_with_string_keys.merge("new_session" => false)).delete rescue nil
          CGI::Session.new(@cgi, session_options_with_string_keys.merge("new_session" => true))
        end
      end
          def stale_session_check!
        yield
      rescue ArgumentError => argument_error
        if argument_error.message =~ %r{undefined class/module (\w+)}
          begin
            Module.const_missing($1)
          rescue LoadError, NameError => const_error
            raise ActionController::SessionRestoreError, "Session contains objects whose class definition isn\'t available. Remember to require the classes for all objects kept in the session. (Original exception: #{const_error.message} [#{const_error.class}])"
          end

          retry
        else
          raise
        end
      end

      def session_options_with_string_keys
        @session_options_with_string_keys ||= ActionController::CgiRequest::DEFAULT_SESSION_OPTIONS.merge(@session_options).inject({}) { |options, (k,v)| options[k.to_s] = v; options }
      end
  end
  
end
end
