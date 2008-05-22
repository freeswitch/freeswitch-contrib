Mime::Type.register "telegraph/voice", :voice
module Telegraph
  def self.config
    rails_config = File.join(RAILS_ROOT, 'config/telegraph')
    if File.exist?(rails_config)
      yml = YAML.load(File.open(File.join(RAILS_ROOT, 'config/telegraph.yml')))
    else
      yml = YAML.load(File.open(File.join(RAILS_ROOT, 'vendor/plugins/telegraph/lib/telegraph.yml')))
    end
    
    config = yml[RAILS_ENV].nil? || yml[RAILS_ENV].empty? ? yml['development'] : yml[RAILS_ENV]
    return config
  end
  
  def self.logger 
    RAILS_DEFAULT_LOGGER
  end
  
  def self.log(txt)
    puts "#{Time.now.utc.to_s(:long)}: #{txt}\n"
  end
end