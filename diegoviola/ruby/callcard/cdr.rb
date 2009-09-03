#!/usr/bin/env ruby

require 'rubygems'
require 'fsr'
require 'fsr/listener/inbound'
require 'database.rb'

def custom_channel_hangup_handler(event)
  event.content.each {|k,v| v.chomp!}
  DB[:cdr].insert(
    :caller_caller_id_name => event.content[:caller_caller_id_name],
    :caller_caller_id_number => event.content[:caller_caller_id_number],
    :caller_destination_number => event.content[:caller_destination_number],
    :caller_context => event.content[:caller_context],
    :variable_start_stamp => event.content[:variable_start_stamp],
    :variable_answer_stamp => event.content[:variable_answer_stamp],
    :variable_end_stamp => event.content[:variable_end_stamp],
    :variable_duration => event.content[:variable_duration],
    :variable_billsec => event.content[:variable_billsec],
    :variable_hangup_cause => event.content[:variable_hangup_cause],
    :unique_id => event.content[:unique_id],
    :core_uuid => event.content[:core_uuid],
    :variable_accountcode => event.content[:variable_accountcode],
    :channel_read_codec_name => event.content[:channel_read_codec_name],
    :variable_write_codec => event.content[:variable_write_codec])
end

FSL::Inbound.add_event_hook(:CHANNEL_HANGUP_COMPLETE) {|event| custom_channel_hangup_handler(event) }

FSR.start_ies!(FSL::Inbound, :host => "localhost", :port => 8021)
