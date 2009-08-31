#!/usr/bin/ruby

require 'rubygems'
require 'fsr'
require 'fsr/listener/outbound'
require 'sequel'

FSR.load_all_commands

DB = Sequel.connect('mysql://root@localhost/callcard')

class Card < Sequel::Model
end

class Destination < Sequel::Model
end

class CallCard < FSR::Listener::Outbound
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
	      @destination = Destination.find(:prefix => prefix)
              FSR::Log.info "*** Success, grabbed #{destination_number} from #{exten}"
              FSR::Log.info "*** Setting up the billing variables."
              uuid_setvar(@session.headers[:unique_id], 'nibble_rate', @destination.rate) if @destination.respond_to?(:rate)
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
    @duration = @card.balance.to_i / @destination.rate.to_i if @destination.respond_to?(:rate)
    return @duration
  end
end

FSR.start_oes! CallCard, :port => 8084, :host => "127.0.0.1"

