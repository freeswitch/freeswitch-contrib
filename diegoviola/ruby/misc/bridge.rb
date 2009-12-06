#!/usr/bin/env ruby

require 'sinatra'
require 'erb'
require 'fsr'

FSR.load_all_commands

get '/' do
  erb "<form action=\"/originate\" method=\"post\"><input type=\"text\" name=\"phone_number\"><input type=\"submit\"></form>"
end

post '/originate' do
  sock = FSR::CommandSocket.new
  sock.originate(:target => "sofia/gateway/gizmo/#{params[:phone_number]}", :endpoint => FSR::App::Conference.new("3030", "default")).run
  #sock.originate(:target => "sofia/internal/#{params[:phone_number]}@65.208.88.11", :endpoint => FSR::App::ExecuteApp.new("playback", "/tmp/200911271756500.fondo4.wav")).run
  redirect '/'
end
