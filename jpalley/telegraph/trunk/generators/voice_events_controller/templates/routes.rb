# Use this to route events to various voice events controller based on parameters sent in the event.
# See example below
class VoiceEventsRouter
  def self.route(map)
    
#    Example of how to connect a route based on params sent my Voice Server
#     map.connect :dialer do |params|
#       MyModel.find_by_uuid(params[:caller_unique_id])  or params[:job_command_arg] =~ /dialers/
#    end
    
    map.default :<%= file_name %>
    
  end
end
