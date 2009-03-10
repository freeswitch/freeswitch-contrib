#!/usr/bin/env ruby

# read SIP messages from a plain text file(FreeSWITCH with TPORT_DUMP=/tmp/xxxx.log),
# and write to a Mysql database
#
# There are lots of ways to monitor sip messages. However, not all of them are convinient
# as we want. Let's say a simple example:
# 
# FreeSWITCH :> originate sofia/gateways/carrier1/5550000|sofia/gateways/carrier2/5550000|sofia/carrier3...
# 
# It's hard to tell what happend if the call fails. Because it's different sip sessions.
# The idea is to group them in one super session and see what happend. I do this by
# adding an arbitary sip header to do cross reference. And by parse the sip messages to 
# a DB we can easily show it as html. I even can build a simple graph based on the DB data:
# 
# http://skitch.com/seven1240/b8xj2/voip-master-idapted
# 
# You can easily add a sip header to INVITE by(I use x_interaction):
# FreeSWITCH :> originate {sip_h_x_interaction=TEST0001}sofia/gateways/.....
# 
# And I can get all the messages from DB:
# SELECT * FROM `sip_messages` WHERE (call_id IN
#         (SELECT distinct call_id FROM sip_messages WHERE x_interaction = 'TEST0001'))  ORDER BY created_at;
# 
# There are two aproches to get sip packets:
# 1) tcpdump/tshark
# 2) FreeSWITCH
# 
# I use the second. Note, there is no way to actually get sip messages from FS currently, but sofia-sip
# has the ability to log all sip messages to a disk file by using TPORT_DUMP
# 
# And you can use this script to parse them to a DB. I know it hurt performance,
# but we don't have tons of traffic and you know there are only 5-10 messages for each
# sip call. While we get about 1G bytes each day in the sip log, most of them are OPTIONS/NOTIFY etc.
# I filtered them before inserting to DB, but it will be better if sofia-sip can filter that :)
# 
# The script will monitor the log file and parse and insert to DB in real time. It's written in
# the Ruby on Rails framework, however, I think it can run out of Rails with or without
# modification. But you still need ruby and rubygems if you want to use it.
# 
# on Ubuntu/Debian
# # apt-get install ruby rubygems
# # gem install mysql file-tail yaml
# 
# It's just an idea, you may like to write your own tools to parse the sip log file. Also
# the log file need to be rotated regularly. And I think it maybe possible to store the log
# file on a memory disk, whatever... :)
#
# By default, it will go to the end of a log file and follow(like tail),
# if you want to parse the existing part, comment line: f.backward(0)
#
# Sample mysql DB DDL:
# CREATE TABLE `sip_messages` (
#   `id` int(11) NOT NULL auto_increment,
#   `message_type` varchar(255) default NULL,
#   `sip_to` varchar(255) default NULL,
#   `call_id` varchar(255) default NULL,
#   `message_bytes` int(11) default NULL,
#   `content_length` int(11) default NULL,
#   `message_body` text,
#   `created_at` datetime default NULL,
#   `x_interaction` varchar(255) default NULL,
#   PRIMARY KEY  (`id`),
#   KEY `index_sip_messages_on_x_interaction` (`x_interaction`)
# ) ENGINE=InnoDB;
#
# Author: seven_at_idapted.com
# Lisense: Free of use

require 'rubygems'
require "file/tail"
require "mysql"
require "yaml"


RAILS_ROOT =  File.expand_path(File.join(File.dirname(__FILE__), '..'))
# Uncomment the following line and comment the above to make it work withouth rails framework
# RAILS_ROOT = "."
DB_CONFIG_FILENAME = File.join(RAILS_ROOT, "config/database.yml")
LOG_FILE = File.join(RAILS_ROOT, 'log/sip2db.log')
# FILENAME="/tmp/tport_sip.log"
FILENAME="/usr/local/freeswitch/log/tport_sip.log"
@debug = true

if File.writable?(LOG_FILE)
  @log_file = File.open(LOG_FILE, "a+")
  puts "Writing logs to #{LOG_FILE}"
else
  @log_file = STDOUT
end

def log(msg)
  @log_file.puts "#{Time.now.strftime("%Y-%m-%d %H:%M:%S")}: #{msg}"
end

def db_config
  if File.readable?(DB_CONFIG_FILENAME)
    conf = YAML.load_file(DB_CONFIG_FILENAME)
    return conf["production"]
  else
    conf["host"] = "localhost"
    conf["username"] = "root"
    conf["password"] = ""
    conf["database"] = "freeswitch_sip_db"
  end
end

#connect to Mysql
@dbh = nil
@debug = true
@call_id_array = [] #use as a queue to filter related messages

def mysql_connect
  Mysql.real_connect(db_config["host"] || db_config["server"], db_config["username"], db_config["password"], db_config["database"])  
end

def ignore_message?(call_id, message_type)     
  #we do not care about certain types of messages and it's related ones
  
  # log @call_id_array.size.to_s                                              
  @call_id_array.shift if @call_id_array.size > 10000
  return true if @call_id_array.include?(call_id)
  if message_type =~ /(REGISTER|OPTIONS|NOTIFY)/
    @call_id_array << call_id
    return true
  end
  return false
end

def insert_to_db(message_body)
  
  # puts message_body
  message_body =~ /(recv|sent) (\d+)[^\n]*\n(\S+).*To: ([^\n]+).*Call-ID: (\S+).*Content-Length: (\d+)[^(x_int)]*(x_interaction: ([A-Z0-9]+))*/m
  message_bytes, message_type, sip_to, call_id, content_length, x_interaction = $2, $3, $4, $5, $6, $8
  sip_to.sub!(/^.*<([^;>]+).*/, '\1') unless sip_to.nil?
  
  return if call_id.nil?
  
  call_id = call_id[0,100] + "_TRUNCATED" if call_id.length > 100
  return if ignore_message?(call_id, message_type)
  
  message_body = @dbh.escape_string(message_body)
  sql = "INSERT INTO sip_messages (message_type, sip_to, call_id, message_bytes, x_interaction, content_length, message_body, created_at)
      VALUES('#{message_type}', '#{sip_to}', '#{call_id}', '#{message_bytes}', '#{x_interaction}', #{content_length || 0 }, '#{message_body}', now())"
  log sql if @debug
    
  begin
    @dbh.query(sql)
  rescue Mysql::Error => e
    log "Error code #{e.errno}"
    log "Error message: #{e.error}"
    log "Error SQLSTATE: #{e.sqlstate}" if e.respond_to?("sqlstate")
  end
end


#start here
begin
  # connect to the MySQL server
  # get server version string and display it
  @dbh = mysql_connect
  log "Server version: " + @dbh.get_server_info
rescue Mysql::Error => e
  log "Error code: #{e.errno}"
  log "Error message: #{e.error}"
  log "Error SQLSTATE: #{e.sqlstate}" if e.respond_to?("sqlstate")
ensure
  # disconnect from server
  # @dbh.close if @dbh
end



File.open(FILENAME) do |f|
  message_type_expecting = true
  message_body = ''
  message_started = false
  call_id = nil
  line_count = 0

   f.extend(File::Tail)
   f.interval = 10
   f.backward(0)
   f.tail do |line|
     
     if line_count > 1000 # crazy?
       line_count = 0
       message_started = false
     end
     
     if !message_started && line =~ /^(recv|sent) (\d+)/
       message_started = true
     end
     
     if line =~ /\013/    # \v - seperator in dump file
       insert_to_db message_body
       message_body = ''
       message_started = false
     else
       message_body << line
     end

  end
end



