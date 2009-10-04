#!/usr/bin/env ruby

require 'rubygems'
require 'database.rb'

DB.create_table :cdr do
  primary_key :id
  String :caller_caller_id_name
  String :caller_caller_id_number
  String :caller_destination_number
  String :caller_context
  String :variable_start_stamp
  String :variable_answer_stamp
  String :variable_end_stamp
  String :variable_duration
  String :variable_billsec
  String :variable_hangup_cause
  String :unique_id
  String :core_uuid
  String :variable_accountcode
  String :channel_read_codec_name
  String :variable_write_codec
end
