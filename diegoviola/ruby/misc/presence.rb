#!/usr/bin/env ruby

require 'rubygems'
require 'fsr'
require "fsr/listener/inbound"

def custom_channel_hangup_handler(event)
  FSR::Log.info "From: #{event.content[:from]} - #{event.content[:status]}"
end

FSL::Inbound.add_event_hook(:PRESENCE_IN) {|event| custom_channel_hangup_handler(event) }
FSL::Inbound.add_event_hook(:PRESENCE_OUT) {|event| custom_channel_hangup_handler(event) }

FSR.start_ies!(FSL::Inbound, :host => "localhost", :port => 8021)
