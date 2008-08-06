require 'xmlrpc/client'
require 'xmlrpc/marshal'
require 'pp'


module Telegraph
  module FreeSWITCH 
  class VoiceModelFunctionError < StandardError; end
  class VoiceModelResponseError < StandardError; end
  
  

  
  class VoiceConnector
    SHOW_COMMANDS = %w{codec application api dialplan file timer calls channels}
    @@connection = nil
    @@connection_options = nil
    
    def self.connect!(opts)
      Telegraph::FreeSWITCH::VoiceModel.new.connect(opts)
    end
    
    def self.load_configuration_and_connect!
      config = Telegraph.config
      Telegraph::FreeSWITCH::VoiceConnector.new.connect!(:server=>config['server'], :port=>config['model']['port'], :username=>config['model']['username'], :password=>config['model']['password'], :timeout=>10)
    end
    
    def connect!(opts={})
      #   opts[:timeout] =|| 10
      @@connection_options = opts
    
      @@connection = XMLRPC::Client.new(@@connection_options[:server], '/RPC2', @@connection_options[:port], nil, nil, @@connection_options[:username], @@connection_options[:password], nil, @@connection_options[:timeout])
    end
  
    def show(command, options='')
      return parse_csv_result(send_command('show', "#{command} #{options}"))
    end
    
    def method_missing(meth, *args)
      send_command(meth.to_s, args.join(' '))
    end
    
    def self.find(command, options='')
      voice_model = Telegraph::FreeSWITCH::VoiceModel.new
      if SHOW_COMMANDS.include?(command.to_s)
        result = voice_model.send_command('show', "#{command} #{options}")
        return voice_model.parse_csv_result(result)
      end
      
    end
    
        
        
    
    def parse_csv_result(result)
      name_array = []
      return_array = []
      result.split("\n").each do |line|
        if line =~ /Content-Type/ or line.empty? or line =~ /\d+ total\./
          #Do nothing
        elsif name_array.empty?
          name_array = line.split(',')
        else
          item = Hash.new
          line.split(',').each_with_index{|v,i| item[name_array[i].to_sym]=v}
          return_array << item
        end
      end
      
      return return_array
    end
          
    def send_command(command, options='')
      result = @@connection.call("freeswitch.api", command, options)
      #Parse results
      if result =~ /ERROR/
        raise VoiceModelFunctionError, "Invalid Command"                              
      elsif result =~ /^-ERR/
        raise VoiceModelResponseError, result.gsub('-ERR', '')
      else
        return (result.gsub('+OK ', '') || true)
      end 
    end
  end
end
end

# puts 'start'
# v = Telegraph::FreeSWITCH::VoiceConnector.new
# puts 'v'
# v.connect!(:server=>'192.168.1.10', :port=>8080, :username=>'freeswitch', :password=>"works")
# puts '2'
# channels = Telegraph::FreeSWITCH::VoiceChannelModel.find(:all)
# pp channels
# 
# channels.first.destroy
# 
# puts '4'