#!/usr/bin/env ruby

require 'rubygems'
require 'fsr'
require 'fsr/listener/inbound'
require 'fsr/listener/outbound'
require 'sequel'

DB = Sequel.connect('mysql://root@localhost/freeswitch')

class Rate < Sequel::Model
end

module Prepaid
  def update_balance
    @@timer = EM::PeriodicTimer.new(1){ puts "hello world" }
  end
end

class Inbound < FSL::Inbound
  include Prepaid

  add_event_hook(:CHANNEL_HANGUP) {|event| custom_channel_hangup_handler(event) }
  def self.custom_channel_hangup_handler(event)
    @@timer.cancel
  end
end

class Outbound < FSR::Listener::Outbound
  include Prepaid

  def session_initiated
    update_balance
  end
end

EM.run do
  EM.connect "0.0.0.0", 8021, Inbound
  EM.start_server "0.0.0.0", 8084, Outbound
end
