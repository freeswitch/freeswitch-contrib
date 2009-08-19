#!/usr/bin/ruby

require 'rubygems'
require 'fsr'
require 'fsr/listener/outbound'
require 'activerecord'

FSR.load_all_commands

ActiveRecord::Base.establish_connection(
  :adapter  => "mysql",
  :host     => "localhost",
  :username => "root",
  :password => "",
  :database => "callcard_development"
)

class Card < ActiveRecord::Base
end

class Destination < ActiveRecord::Base
end

class CallCard < FSR::Listener::Outbound
  def session_initiated
    exten = @session.headers[:caller_caller_id_number]
    pin_wav = "/usr/local/freeswitch/sounds/en/us/callie/conference/8000/conf-pin.wav"
    bad_pin_wav = "/usr/local/freeswitch/sounds/en/us/callie/conference/8000/conf-bad-pin.wav"
    dial_tone = "tone_stream://%(10000,0,350,440)"
    FSR::Log.info "*** Answering incoming call from #{exten}"
    play_and_get_digits(pin_wav, bad_pin_wav, 2, 10, 3, 7000, ["#"], "pin_number", "\\d") do |pin_number|
      @card = Card.find_by_card_number(pin_number)
      if @card
	FSR::Log.info "*** Success, grabbed #{pin_number} from #{exten}"
	play_and_get_digits(dial_tone, bad_pin_wav, 2, 11, 3, 7000, ["#"], "destination_number", "\\d") do |destination_number|
	  prefix = destination_number[0,5]
	  @destination = Destination.find_by_prefix(prefix)
	  FSR::Log.info "*** Success, grabbed #{destination_number} from #{exten}"
	  FSR::Log.info "*** Setting up the billing variables."
	  uuid_setvar(@session.headers[:unique_id], 'nibble_rate', @destination.rate) if @destination.respond_to?(:rate)
	  uuid_setvar(@session.headers[:unique_id], 'nibble_account', @card.id)
	  close_connection
	end
      else
	FSR::Log.info "*** Failure, grabbed #{pin_number} from #{exten}"
	playback(bad_pin_wav)
	FSR::Log.info "*** Hanging up the call."
	hangup
      end
    end
  end
  
  def duration
    @duration = @card.balance.to_i / @destination.rate.to_i if @destination.respond_to?(:rate)
    return @duration
  end
end

FSR.start_oes! CallCard, :port => 8084, :host => "127.0.0.1"

