#!/usr/bin/env ruby

require 'ramaze'
require 'fsr'

FSR.load_all_commands

class Main < Ramaze::Controller
  engine :ERB

  def index
    "Phone number: <form action=\"/originate\" method=\"post\"><input type=\"text\" name=\"phone_number\"><input type=\"submit\"></form>"
  end

  def originate
    sock = FSR::CommandSocket.new
    sock.originate(:target => "sofia/gateway/flowroute/#{request[:phone_number]}", :endpoint => FSR::App::Conference.new("3030", "default")).run
    redirect '/'
  end
end

Ramaze.start

