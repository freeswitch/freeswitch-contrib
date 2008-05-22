#Core modules
require 'core/telegraph'

#Load the voice model
require 'freeswitch/voice_model/voice_connector'
require 'freeswitch/voice_model/voice_channel_model'
require 'freeswitch/voice_model/voice_sip_model'
require 'freeswitch/voice_model/voice_conference_model'

#Connect to server...
Telegraph::FreeSWITCH::VoiceConnector.load_configuration_and_connect!

