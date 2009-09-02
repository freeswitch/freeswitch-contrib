#!/usr/bin/env ruby

require 'rubygems'
require 'fastercsv'
require 'database.rb'

FasterCSV.foreach('outbound_rates.csv') do |row|
  DB[:rates].insert(:destination => row[0], :prefix => row[1], :first_increment => row[2], :second_increment => row[3], :rate => row[4])
end

