class VoiceModelGenerator < Rails::Generator::NamedBase
  def initialize(runtime_args, runtime_options = {})
    super
    @model_type = runtime_args.shift.downcase
    @model_name = runtime_args.shift
  end
  def manifest
    raise "Invalid Voice Model Name" unless %w{channel sip conference}.include?(@model_type)
    
    record do |m|
      m.directory File.join('app/models')
      m.template "model.rb", File.join('app/models', "#{@model_name.tableize.singularize}.rb"), :assigns=>{:model_type=>@model_type, :model_name=>@model_name}
    end
  end
end
