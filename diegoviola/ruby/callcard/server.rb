#!/usr/bin/env ruby

require 'rubygems'
require 'fsr'
require 'fsr/listener/outbound'
require 'fsr/listener/inbound'
require 'database.rb'

FSR.load_all_commands

class CDR < FSL::Inbound
  def start
    add_event_hook(:CHANNEL_HANGUP_COMPLETE) {|event| custom_channel_hangup_handler(event)}
  end

  def self.custom_channel_hangup_handler(event)
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
end

class Card < Sequel::Model
end

class Rate < Sequel::Model
end

class Core < FSR::Listener::Outbound
  def session_initiated
    exten = @session.headers[:caller_caller_id_number]
    pin_wav = "/usr/local/freeswitch/sounds/en/us/callie/conference/8000/conf-pin.wav"
    bad_pin_wav = "/usr/local/freeswitch/sounds/en/us/callie/conference/8000/conf-bad-pin.wav"
    dial_tone = "tone_stream://%(10000,0,350,440)"
    FSR::Log.info "*** Answering incoming call from #{exten}"
    answer do
      fs_sleep(2000) do
        play_and_get_digits(pin_wav, bad_pin_wav, 2, 10, 3, 7000, ["#"], "pin", "\\d") do |pin|
	  @card = Card.find(:card_number => pin)
          if @card
            FSR::Log.info "*** Success, grabbed #{pin} from #{exten}"
            play_and_get_digits(dial_tone, bad_pin_wav, 2, 11, 3, 7000, ["#"], "destination_number", "\\d") do |destination_number|
              prefix = destination_number[0,5]
	      @rate = Rate.find(:prefix => prefix)
              FSR::Log.info "*** Success, grabbed #{destination_number} from #{exten}"
              FSR::Log.info "*** Setting up the billing variables."
              uuid_setvar(@session.headers[:unique_id], 'nibble_rate', @rate.rate) if @rate.respond_to?(:rate)
              uuid_setvar(@session.headers[:unique_id], 'nibble_account', @card.id)
              FSR::Log.info "*** Bridging."
              FSR::Log.info "*** You have #{duration} minutes to talk."
              speak("You have #{duration} minutes to talk.")
              transfer(destination_number, "XML", "default") { close_connection }
              #bridge("sofia/internal/#{destination_number}@0.0.0.0")
            end
          else
            FSR::Log.info "*** Failure, grabbed #{pin} from #{exten}"
            playback(bad_pin_wav)
            FSR::Log.info "*** Hanging up the call."
            hangup
          end
        end
      end
    end
  end
  
  def duration
    duration = @card.balance / @rate.rate if @rate.respond_to?(:rate)
    return duration
  end
end

FSR.start_oes! Core, :port => 8084, :host => "127.0.0.1"

