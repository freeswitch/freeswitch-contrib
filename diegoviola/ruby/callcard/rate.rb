#!/usr/bin/ruby

require 'rubygems'
require 'fastercsv'
require 'sequel'

DB = Sequel.connect('mysql://root@192.168.0.2/callcard')

FasterCSV.foreach('outbound_rates.csv') do |row|
  DB[:destinations].insert(:country => row[0], :prefix => row[1], :rate => row[4])
end

