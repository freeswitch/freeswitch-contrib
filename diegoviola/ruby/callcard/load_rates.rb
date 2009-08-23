#!/usr/bin/ruby

require 'rubygems'
require 'fastercsv'
require 'activerecord'

ActiveRecord::Base.establish_connection(
  :adapter  => "mysql",
  :host     => "localhost",
  :username => "root",
  :password => "",
  :database => "callcard_development"
)

class Destination < ActiveRecord::Base
end

FasterCSV.foreach(ARGV.to_s) do |row|
  destination = Destination.new(:country => row[0], :prefix => row[1], :rate => row[4])
  destination.save
end

