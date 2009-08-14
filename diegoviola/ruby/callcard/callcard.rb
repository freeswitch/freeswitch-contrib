#!/usr/bin/ruby

require 'rubygems'
require 'fsr'
require 'fsr/listener/outbound'
require 'activerecord'

FSR.load_all_commands

class Card < ActiveRecord::Base
  ActiveRecord::Base.establish_connection(
    :adapter  => "mysql",
    :host     => "localhost",
    :username => "root",
    :password => "",
    :database => "callcard_development"
  )
end

class Destination < ActiveRecord::Base
  ActiveRecord::Base.establish_connection(
    :adapter  => "mysql",                 
    :host     => "localhost",             
    :username => "root",                  
    :password => "",
    :database => "callcard_development"
  )
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
        play_and_get_digits(pin_wav, bad_pin_wav, 2, 10, 3, 7000, ["#"], "pin_number", "\\d") do |pin_number|
          @card = Card.find_by_card_number(pin_number)
          if @card then
            FSR::Log.info "*** Success, grabbed #{pin_number} from #{exten}"
            play_and_get_digits(dial_tone, bad_pin_wav, 2, 11, 3, 7000, ["#"], "destination_number", "\\d") do |destination_number|
              prefix = destination_number[0,5]
              @destination = Destination.find_by_prefix(prefix)
              FSR::Log.info "*** Success, grabbed #{destination_number} from #{exten}"
              FSR::Log.info "*** Setting up the billing variables."
              uuid_setvar(@session.headers[:unique_id], 'nibble_rate', @destination.rate)
              uuid_setvar(@session.headers[:unique_id], 'nibble_account', @card.id)
              destination_info
              FSR::Log.info "*** Bridging."
              FSR::Log.info "*** You have #{duration} minutes to talk."
              speak("You have #{duration} minutes to talk.")
              transfer("#{destination_number}", "XML", "default") { close_connection }
	      #api("sched_api +#{(duration*60)-60} none uuid_displace #{@session.headers[:unique_id]} start tone_stream://%(500,0,500)")
              #bridge("sofia/internal/#{destination_number}@0.0.0.0")
            end
          else
            FSR::Log.info "*** Failure, grabbed #{pin_number} from #{exten}"
            playback(bad_pin_wav)
            FSR::Log.info "*** Hanging up the call."
            hangup
          end
        end
      end
    end
  end
  
  def duration
    @duration = @card.balance.to_i / @destination.rate.to_i
    return @duration
  end

  def destination_info
    FSR::Log.info "*** Destination rate: #{@destination.rate}"
    FSR::Log.info "*** Card ID: #{@card.id}"
    FSR::Log.info "*** Calling to: #{@destination.country}"
  end

  def unbind
    FSR::Log.info "*** Disconnected."
  end
end

FSR.start_oes! CallCard, :port => 8084, :host => "127.0.0.1"

