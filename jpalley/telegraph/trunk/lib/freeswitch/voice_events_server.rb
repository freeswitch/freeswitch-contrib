Dir[File.join(RAILS_ROOT, "app/voice_events/**")].each do |f|
  require f
end

module Telegraph
  module FreeSWITCH
    class VoiceEventsServer < EventMachine::Connection

      def initialize *args

        @received_buffer = ''
        @last_command = nil
        @closing = false
        @routes = Telegraph::Core::Router.new
        VoiceEventsRouter.route(@routes)
        super
      end
      
      def set_modes(verbose, debug)
        Telegraph.log "Turning on Debug Mode" if  @debug = debug
        Telegraph.log "Turning on Verbose Mode" if @verbose = verbose
      end
      
        
      def receive_data(data)
        # IMPORTANT: There is no gauruntee where data starts/ends
        # We must make sure we buffer correctly
        @received_buffer << data

        Telegraph.log "Recieved:\n#{data}" if @verbose
        #Return unless we have a complete event
        process_event
        
     
          
        while @received_buffer =~  /Content-Type:\s([\w\/]+)/
          content_type = $1
          after_buffer = $'
          
          #Is this an authorizatin reqest?
          if content_type == 'auth/request'
            @received_buffer = after_buffer
            send_command "auth ClueCon"
            
          #Reply to Authorization request?
          elsif content_type == 'command/reply'
            @received_buffer = ''
            if after_buffer.include?('Reply-Text: +OK accepted')
             send_command "event plain #{subscribed_events}"
            end
              
          #Everything else
          else
            #Search for content-length/content-type tags and extra the values
            while @received_buffer =~ /Content-Length:\s(\d+)\nContent-Type:\s([\w\/\-]+)/
              content_type = $2
              @next_event_size = $1.to_i
              
              @received_buffer = $'.gsub(/\A\s+/, '') #Remove leading whitespaces. 
              
              process_event
            end
          end
        end
          
      end
      
      def process_event
        if @next_event_size and @received_buffer and @received_buffer.size >= @next_event_size
            @event_data = @received_buffer[0..(@next_event_size-1)]
            @recieved_buffer = @received_buffer[@next_event_size..(@received_buffer.size-1)]
            @next_event_size = nil

            #Split at content length
            data = @event_data.split(/Content-Length:\s(\d+)\n\n/)
            
           headers = Hash.new
            data[0].split("\n").each do |line| 
              unless line.empty?
                k, v = line.split(': ')
                headers[k.gsub('-', '_').downcase.to_sym] = URI.decode(v) if k and v
              end
            end
            
            headers[:content] = data[2] if data[2]

            event_name = headers[:event_name].downcase
             event_name = "custom_" + headers[:event_subclass].gsub('::', '_') if headers[:event_name] == "CUSTOM"
            begin
               return unless VoiceEventsRouter.filter(headers)
               Telegraph.log "CALLING: #{event_name}"
               @routes.dispatch!(event_name.to_sym, headers)
               # @voice_events.params = headers
               # @voice_events.send(event_name.to_sym)
             rescue ActiveRecord::StatementInvalid => e
               #If not used for a while we will lose AR Connection
               Telegraph.log "Lost AR Connection .... "
               ActiveRecord::Base.connection.reconnect!
               unless @ar_reconnect_retried
                 Telegraph.log "Reconnecting to AR..."
                 @ar_reconnected_retried = true
                 retry
               end
               raise
             rescue Exception => e
               Telegraph.log "Error in sending event: #{headers[:event_name]}"
               Telegraph.log e.inspect
               Telegraph.log e.backtrace
             else  
              @ar_reconnected_retried = false
             
             end
          
        end
      end
      def unbind
        return if @closing
        
        Telegraph.log "UNBIND!"

        begin
          sleep 10
          Telegraph.log "Attempting to reconnect..."
          config = Telegraph.config
          reconnect config['server'], config['events']['port']
        rescue SystemExit => e
          Telegraph.log "system exit"
          exit
        rescue Exception => e
          Telegraph.log "Error in reconnecting..."
          Telegraph.log e.inspect

          retry
        end
        
      end
      
      protected
      def subscribed_events
        return "all" if @debug

        custom_events = []
        normal_events = []

        @routes.used_events.each do |e|
          e = e.downcase
          if e =~ /custom\_/ then
            custom_events << $'.gsub('_', '::')
          else
            normal_events << e.upcase
          end
        end
        
        meths = normal_events.join(' ')
        meths << " CUSTOM #{custom_events.join(' ')}"
        Telegraph.log "Subscribe to: #{meths}"
        return meths
      end
      
      def send_command(str)
        @last_command = str
        send_data(str + "\n\n")
      end
    end
  end
end