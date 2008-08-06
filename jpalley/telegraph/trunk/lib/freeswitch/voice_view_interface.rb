module Telegraph
  module FreeSWITCH
    class VoiceViewInterface < Telegraph::VoiceView::Interface
      #The following is the interface to the freeswitch voice view functions...
      attr_accessor :params, :commands
      
      def initialize(params)
        ENV['REQUEST_METHOD'] = "post"
        puts "INITIALIZE VOICE"
        @params = params
        @commands = Array.new
        @cgi=CGI.new
      end
  
      def url
        '/' + (@params['variable_socket_path'] || '')
      end
  
      #spell macro
      def spell(words)
        phrase "spell", words
      end
  
      #spell phonetic macro
      def spell_phonetic(words)
        phrase "spell-phonetic", words
      end
  
      #say time macro
      def say_time(time = Time.now.utc)
        phrase "saydate", time.to_i
      end
  
      #say time difference
      def say_timespec(time)
        phrase "timespec", time
      end
  
      #answer channel
      def answer
        msg "answer"
      end
  
      def play_sound(file)
        msg "playback", file
      end
  
      def record(filename, length, silence_threshold=500, silence_time=5)
        msg "record", "#{filename} #{length} #{silence_threshold} #{silence_time}"
      end
  
      def phrase(macro, data)
        msg "phrase", "#{macro},#{data}"
      end
    
      def bridge(data)
        msg "bridge", data
      end
      
      def set(data)
        msg "set", data
      end
      
      def conference(data)
        msg "conference", data
      end
      
      def hangup
        @commands << "SendMsg #{params['unique-id']}\ncall-command: hangup\nhangup-cause: SERVICE_NOT_IMPLEMENTED"
      end
      
      def hold
        @commands << "SendMsg #{params['unique-id']}\ncall-command: hold"
      end
      
      def ring_ready
        msg "ring_ready"
      end
      
      def socket(url)
        msg "socket", "#{params['variable_socket_host']}:8084#{url} async full"
      end
      
      def fifo_in(queue_name, announce = "undef", music="undef")
        msg "fifo", "#{queue_name} in #{announce} #{music}"
      end
      
      def fifo_out(queue_name, announce="undef", music="undef")
        msg "fifo", "#{queue_name} out nowait #{announce} #{music}"
      end
      
      def export(val)
        msg "export", val
      end
      
      def speak(txt, engine="flite", voice="kal")
  		  msg "set", "tts_engine=#{engine}"
  		  msg "set", "tts_voice=#{voice}"
        msg "speak", txt
      end
      
      def transfer(extension, dialplan='', context='')
        msg "transfer", "#{extension} #{dialplan} #{context}"
      end
      def dtmf_read(max = 10, sound = nil, fs_var_name = "dtmf_inputs", timeout = 10000, terminators = '#,*')
        sound ||= 'misc/8000/silence.wav'
        msg "read", "1 #{max} #{sound} #{fs_var_name} #{timeout} #{terminators}"
      end
      
      ##############
      # For FORMS
      ##################
      def build_form_element(element)
        dtmf_read(element[:max_digits], element[:sound], element[:param], element[:timeout],  element[:terminators]) if element[:type]==:get_dtmf
      end
      
      def create_redirect(url, method="GET")
        set("http_method=#{method}")
         transfer "'socket:${rails_server}#{url} async full'", "inline"
      end
      private
      def msg(app, params='')
        @commands << {:type=>:msg, :app=>app, :params=>params}
      end
    end
  end
end