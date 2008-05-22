module Telegraph
  module FreeSWITCH
    class VoiceChannelModel
      attr_accessor :exists
      attr_accessor :last_error
      attr_accessor :job_uuid
      attr_accessor :uuid
      
      @uuid = nil
      @params = nil
      @@connector = nil
      @exists = nil
      @last_error = nil
      def initialize(params={})
        @params = params
        @uuid = params[:uuid]
        @job_uuid = params[:job_uuid]
        @@connector = params[:connector] || VoiceConnector.new
        @valid=true
      end

      %w{park hold}.each do |command|
        class_eval <<-END
          def #{command}()
            puts @@connector.send_command("uuid_#{command}",@uuid)
          end
        END
      end

      def destroy
        puts @@connector.send_command("uuid_kill", @uuid)
      end

      def invalidate
        @valid = false
      end
      
      def valid?
        @valid
      end

      def start_recording(filename)
         @@connector.send_command('uuid_record', "#{@uuid} start #{filename}")
        
      end
      def stop_recording(filename)
         @@connector.send_command('uuid_record', "#{@uuid} stop #{filename}")
      end
      
      def inline_transfer(apps)
         @@connector.send_command('uuid_transfer', "#{uuid} #{apps} inline")
      end
      
      def dump
        begin
          
          dump=@@connector.send_command('uuid_dump', @uuid)
        
          result = {}
          dump.split("\n").each do |line|
            l = line.split(': ')
            result[l[0].gsub('-', '_').downcase] = result[l[1]]
          end
          return result
       rescue Exception => e
        return nil
       end
      end
      
      def method_missing(method, *args)
        properties = self.dump
        properties[method] ? properties[method] : properties["channel_#{method}"]
      end
      

      def self.humanize_error_code(status)
        case status
        when /NO_ROUTE_DESTINATION|CHAN_NOT_IMPLEMENTED/
          "Invalid Number.  Perhaps the user is not logged in?"
        when /NO_USER_RESPONSE|ORIGINATOR_CANCEL/
          "The phone was not answered."
        when /USER_BUSY/
          "The number is busy now.  Please try again later."
        when /INVALID_NUMBER_FORMAT/
          "The number is not a valid format.  Please enter another number and try again."
        else
          status 
        end
      end
      
      def self.find(meth, *args)
        voice_connector = @@connector || VoiceConnector.new
        if meth == :all
          channels_csv = voice_connector.show(:channels)
          if args.empty?
            return channels_csv.map{|c| VoiceChannelModel.new(c, voice_connector)}
          end
        else
          klass = self.new(:uuid=>meth)
          if klass.dump
            return klass
          else
            return nil
          end
        end
      end
    
      def self.create(opts={})
        voice_connector = @@connector || VoiceConnector.new
        
        destination = opts[:destination]
        cid_name = opts[:cid_name] || 'idapted'
        cid_number = opts[:cid_number] || '000'
        variables = opts[:vars] || {}
        variables[:origination_caller_id_name] = cid_name
        variables[:origination_caller_id_number] = cid_number
        variables[:ignore_early_media] = "true"
        
        variables = variables.map{|k,v| "#{k}=#{v}"}.join(',')

        if callback = opts[:callback]
          port = opts[:port] || '8084'
          #Remove http://blah/ if it exists
          callback.gsub!(/http:\/\/[\d\w\.:]+\//, '')
          params = "&socket(${rails_server}:#{port}/#{callback} async full)"
        else
          params = opts[:params]
        end
        
        begin
          command = "originate {#{variables}}#{destination} #{params}"
          job = voice_connector.send_command('bgapi', command)
          job_uuid = job.gsub('Job-UUID: ', '').gsub("\n", '')
          return self.new :job_uuid=>job_uuid, :connector=>voice_connector
        rescue Exception => e

          mdl = self.new
          mdl.last_error = e.message
          mdl.invalidate
          return mdl
        end
      end
    end
    
  end
end