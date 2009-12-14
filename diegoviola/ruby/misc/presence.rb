#!/usr/bin/ruby

require 'rubygems'
require 'fsr'
require "fsr/listener/inbound"

def presence_handler(event)
  FSR::Log.info "From: #{event.content[:from]} - #{event.content[:status]}"
end

# Event hooks for presence.
FSL::Inbound.add_event_hook(:PRESENCE_IN) {|event| presence_handler(event) }
FSL::Inbound.add_event_hook(:PRESENCE_OUT) {|event| presence_handler(event) }

FSR.start_ies!(FSL::Inbound, :host => "localhost", :port => 8021)
