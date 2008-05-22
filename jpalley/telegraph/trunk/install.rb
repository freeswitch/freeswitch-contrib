# Copy server files into script/server
require 'ftools'

dir = File.dirname(__FILE__)

#Copy script files
%w{voice_view voice_events}.each do |f|
  File.copy "#{dir}/lib/#{f}", "#{dir}/../../../script/#{f}"
end

#Copy configuration
File.copy "#{dir}/lib/telegraph.yml", "#{dir}/../../../config/telegraph.yml"