class <%= class_name %>VoiceEvents < Telegraph::Core::VoiceEventsBase
# Common events are commented below

# Background job is called as a result of VoiceChannelModel.create and will have the uuid of the call
#  def background_job
#  end
  
# Called on hangup
#  def channel_hangup
#  end
  
# Called when two channels are bridged
#  def channel_bridge
#  end
  
# When a sip user registers (watch out..every time the register is called this fires..not just the first time)
#  def custom_sofia_register
#  end
  
# When a sip user unregisters
#  def custom_sofia_unregister
#  end
  
# When a sip user is expired.
#  def custom_sofia_expire
#  end
  
end
