module Telegraph
  module FreeSWITCH
    class VoiceConferenceModel
      attr_accessor :name
      attr_accessor :members
      @name = nil
      @members = nil
      @@connector = nil
      def initialize(name, members_data)
        return nil if members_data =~ /not found/
        @name = name
        @@connector ||= VoiceConnector.new
        @members = VoiceConferenceMemberCollection.new
        members_data.split("\n").each do |member_data|
          @members << VoiceConferenceMember.new(member_data, name, @@connector)
        end
      end
      
      def start_recording(filename)
        @connector.send_command('conference', "#{@name} record #{filename}")
      end

      def stop_recording(filename)
        @connector.send_command('conference', "#{@name} norecord #{filename}")
      end
      
      def self.find(id)
         @@connector ||= VoiceConnector.new

        if id == :all
          data = @@connector.send_command('conference', 'list')
          #TODO: Deal with no conference
          conferences_data = data.split(/Conference\s([\d\w]+)\s[\w\s\d\(]+\)\n/)
          conferences_data = conferences_data[1..conferences_data.size-1] #Pop off first one..its useless
          conferences = []
          while not conferences_data.empty?
            members = conferences_data.pop
            name = conferences_data.pop
            conferences << self.new(name, members)
          end
          return conferences
        else
          data = @@connector.send_command('conference', "#{id} list")
          return nil if data =~ /not found/
          return self.new(id, data)
      end
      end
    end
    
    class VoiceConferenceMemberCollection < Array
      def find(id)
        self.detect{|o| o.id.to_s == id.to_s}
      end
    end
    class VoiceConferenceMember
      attr_accessor :id, :channel, :uuid, :deaf, :mute, :volume_in, :volume_out, :energy, :conf_name
      
      def initialize(data, conf_name, voice_connector)
        data = data.split(';')
        @id = data[0]
        @channel = data[1]
        @uuid = data[2]
        @mute = !(data[5] =~ /speak/) and true
        @deaf = !(data[5] =~ /hear/) and true
        @volume_in = data[6]
        @volume_out = data[7]
        @energy = data[8]
        @conf_name = conf_name
        @connector = voice_connector
      end
      
      def mute?
        @mute
      end
      
      def deaf?
        @deaf
      end
      
      def update_attributes(params)
        puts "UPDATE"
        pp params
        params.each do |k,v|
          puts "param"
          puts k.to_sym
          if [:mute, :deaf].include?(k.to_sym)
            tru = (v and not v == "0")
            prefix = tru ? '' : 'un'
            unless tru == self.send(k.to_sym)
              if @connector.send_command('conference', "#{conf_name} #{prefix}#{k.to_s} #{id}")
                self.send("#{k.to_s}=".to_sym, v)
              end
            end
          elsif [:volume_in, :volume_out, :energy].include?(k.to_sym)
            puts "in valume"
            if @connector.send_command('conference', "#{conf_name} #{k.to_s} #{id} #{v}")
              self.send("#{k}=", v)
            end
          end
        end
      end
            
    end
  end
end