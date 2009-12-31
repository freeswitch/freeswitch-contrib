#!/usr/bin/env ruby

require 'sinatra'
require 'fsr'

FSR.load_all_commands

get '/' do
  "Phone number: <form action=\"/originate\" method=\"post\"><input type=\"text\" name=\"phone_number\"><input type=\"submit\"></form>"
end

post '/originate' do
  sock = FSR::CommandSocket.new
  sock.originate(:target => "sofia/gateway/flowroute/#{params[:phone_number]}", :endpoint => FSR::App::Conference.new("3030", "default")).run
  redirect '/'
end
