module Telegraph
  module VoiceView
    class Form
       attr_accessor :elements
       def initialize
         @elements = Array.new
       end

       def numeric_input(param, pars={})
         @elements << {:type=>:get_dtmf, :sound=>pars[:sound], :param=>param, :timeout=>pars[:timeout] || 5000, :max_digits=>pars[:max_digits] || 7, :terminators=>pars[:terminators]}
       end

       def submit(args)
         @elements << {:type=>'submit', :args=>args}
       end

       def record_input(label, filename, param, max_time=10, beep=true, silence_detect=10)
         @elements << {:type=>'record_input',:label=>label, :filename=>filename,:param=>param, :max_time=>max_time, :beep=>beep, :silence_detect=>silence_detect}
       end
     end
    class Interface
      attr_accessor :controller
      
      def redirect_to(args)
        create_redirect @controller.url_for(args.merge(:only_path=>true))
      end
      

      # The following has not been updated/tested with FreeSWITCH.
      # It should work with minor modifications for someone who needs the features
      
      
      # def play(str)
      #   str.split(' ').each {|e| say_element(e)}
      # end
      # 
      def form (opts={})
          form = Telegraph::VoiceView::Form.new
          @params = {}
          yield form
        
          for element in form.elements
            build_form_element(element)
          end
        
          url = opts[:url] || opts
          url = @controller.url_for(args.merge(:only_path=>true)) unless url.is_a?(String)
          method = opts[:method].to_s.upcase || "POST"
          create_redirect url, method
        end
      # 
      # def extract_hash(key, value)
      #   split_key=key.split('[')
      #   if split_key.size == 1 then
      #     {key.to_sym => value}
      #   else
      #     {split_key[0].to_sym => {split_key[1].slice!(0..(split_key[1].size-2)).to_sym => value}}
      #   end
      # end
      # 
      # def nested_merge(original, added)
      #    added.each{|key,value| 
      #       if value.is_a?(Hash) && original[key] && original[key].is_a?(Hash)
      #         original[key] = nested_merge original[key], value
      #       else
      #         original[key] = value
      #       end   
      #    }
      #   original
      # end
      # 
      # def link_to_dtmf(sound, args={}, &block)
      #   @links = Array.new
      # 
      #   instance_eval &block
      # 
      #   return if @links.empty?
      # 
      #   max_digits = @links.max{|a, b| a[:link].to_s.to_i.to_s.length <=> b[:link].to_s.to_i.to_s.length}[:link].to_s.length
      #   args[:timeout] ||= max_digits * 1500
      # 
      #   n= get_data full_sound_path(sound), args[:timeout], max_digits
      # 
      #   input = n.length == 0 ? 'no_response' :  n.to_s
      # 
      #   #find the matching link
      #   @links.each do |link|
      #     if link[:link].to_s == input
      #       @request.create_redirect link
      #       return
      #     end
      #   end
      # 
      #   #we got nothing
      #   default = @links.detect{|l| l[:link] == :default}
      #   @request.create_redirect default if @request.next_action.nil? and default
      # end
      # 
      # def link(link, url)
      #   url[:link] = link
      #   @links  << url
      # end
      # 
      # 
      # def redirect_to(opts={})
      #   @request.create_redirect opts
      # end 
      # 
      # def say_datetime(time,escapeDigits=ALL_SPECIAL_DIGITS)
      #   #calc the number of seconds elapsed since epoch (00:00:00 on January 1, 1970) 
      #   diff = time.to_i
      #   msg = "SAY DATETIME #{diff} #{escape_digit_string(escapeDigits)}"
      #   send(msg)
      #   return get_int_result()
      # end
      # 
      # def say_element(item)
      #   if item.match(/^Datetime:/) then
      #     item.delete!('Datetime:')
      #     say_datetime(item)
      #   elsif item.to_i > 0 || item=='00'
      #     say_number(item.to_i.to_s)
      #   elsif item.to_f < 0 then
      #     play_sound 'negative'
      #     say_number(item.to_f.abs)
      #   else
      #     play_sound(full_sound_path(item))
      #   end
      # end
      # 
      # def build_form_element(element)
      #   return get_data_element(element) if element[:type] == 'get_data'
      #   return record_input_element(element) if element[:type] == 'record_input'
      # end
      # 
      # def get_data_element(element)
      #   num = get_data full_sound_path(element[:sound]), element[:timeout], element[:max_digits]
      #   @params.merge!(extract_hash(element[:param], num))
      # end
      # 
      # 
      # def record_input_element(element)
      #   play element[:label]
      #   ret = record_file(Telegraph::Config::Globals["RECORDINGPATH"] + '/' + element[:filename], element[:max_time], element[:beep], element[:silence_detect])
      # 
      #   @params = nested_merge(@params, extract_hash(element[:param],element[:filename]))
      #   #params.update(extract_hash(element[:param],element[:filename]))
      # end
      # 
      # def output(filename)
      #   play_sound full_sound_path(filename)
      # end
      # 
      # def full_sound_path(filename)
      # 
      #   name =Telegraph::Config::Globals["SOUNDPATH"] + '/' + filename
      #   if File.exists?(name + '.gsm') || File.exists?(name + '.sln')
      #     name
      #   else
      #     filename
      #   end
      # end
      
    end
  end
end