#!/usr/bin/env ruby

require 'rubygems'
require 'database.rb'

DB.drop_table :cards
DB.drop_table :rates
DB.drop_table :cdr
