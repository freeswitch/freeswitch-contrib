#!/usr/bin/ruby

require 'rubygems'
require 'csv'
require 'sequel'

DB = Sequel.connect('mysql://root@localhost/callcard')

CSV.foreach('outbound_rates.csv') do |row|
  DB[:rates].insert(:destination => row[0], :prefix => row[1], :first_increment => row[2], :second_increment => row[3], :rate => row[4])
end

