class VoiceEventsControllerGenerator < Rails::Generator::NamedBase
  def manifest
    puts self.class_name
    record do |m|
      m.directory File.join('app/voice_events')
      routes_path = File.join('app/voice_events', 'routes.rb')
      m.template ("routes.rb", routes_path) unless File.exists?(routes_path)
      m.template "voice_events.rb", File.join('app/voice_events', "#{file_name}_voice_events.rb")
    end
  end
end
