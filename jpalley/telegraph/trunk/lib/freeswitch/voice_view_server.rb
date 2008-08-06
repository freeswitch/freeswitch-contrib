require 'eventmachine'

   module VoiceViewServer
     def post_init
       Telegraph.log "New Connection"
       @receive_buffer = ''
       @connection_params = []
       @command_queue = []
       @command_queue <<    "log DEBUG\n\n"
       @command_queue << "myevents\n\n"
          
       send "connect"
     end
     
     def set_modes(verbose, debug)
       Telegraph.log "Turning on Debug Mode" if  @debug = debug
       Telegraph.log "Turning on Verbose Mode" if @verbose = verbose
     end
      
     def receive_data data
       @receive_buffer << data
       
       Telegraph.log "Received:\n#{data}" if @verbose
       
       #If we've recieved the last line of the response
       if @receive_buffer =~ /\n\n/ 
         #Hashify params and clear buffer
         @params = hashify @receive_buffer
         @receive_buffer = ''
         
         #Set connection params if this is the first response
         unless @command_queue.empty?
            command = @command_queue.shift
            if command.is_a?(Hash)
              send_application command[:app], command[:params]
            else
              send command
            end
          else
            #close_connection
          end
          
          if @connection_params.empty?
            @connection_params = @params
         
            make_commands
          end
 
        end
     end
     
     def make_commands
       interface = Telegraph::FreeSWITCH::VoiceViewInterface.new(@connection_params)
       @command_queue = @command_queue + Telegraph::VoiceView::Dispatcher.new.dispatch(interface)
      end

     def unbind
       puts "UNBIND"
     end
     
     def hashify data
       hsh = Hash.new
       data.split("\n").each do |line|
         unless line.empty?
           key,value = line.split(': ')
           key = key.gsub('-', '_').downcase
         
           hsh[key] = value
           non_variable_key = key.gsub(/^variable\_/, '') #remove variable_ from keys
           hsh[non_variable_key] = value
          end
       end
       return hsh.with_indifferent_access
    end
    
     def send_application app, params=nil
       msg = "SendMsg #{@connection_params['unique-id']}\ncall-command: execute\nexecute-app-name: #{app}"
       msg << "\nexecute-app-arg: #{params}" #if params
       msg << "\nevent-lock: true"
       send msg
     end
     
     def send data
       Telegraph.log "Sending:\n#{data}" if @verbose
       send_data data + "\n\n"
      end
   end