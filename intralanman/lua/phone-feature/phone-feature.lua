local dbh = freeswitch.Dbh("pgsql://dbname=freeswitch user=freeswitch password='freeswitch'")
assert(dbh:connected())

record_count = 0;

function tf_value( dbval )
   if dbval == "t" then
      return "true"
   else
      return "false"
   end
end


function count_callback( row )
   record_count = row["record_count"]
end


function init_callback( row )
   freeswitch.console_log("INFO", "init_callback\n")
   print_r(row)
   
   local e = freeswitch.Event("SWITCH_EVENT_PHONE_FEATURE")
   e:addHeader("profile", row["profile_name"])
   e:addHeader("user", row["username"])
   e:addHeader("host", row["domain"])
   e:addHeader("device", row["username"])
   e:addHeader("Feature-Event", "init")
   e:addHeader("doNotDisturbOn", tf_value(row["dnd"]))
   e:addHeader("forward_immediate", row["cfwd_immediate_number"])
   e:addHeader("forward_immediate_enabled", tf_value(row["cfwd_immediate_active"]))
   e:addHeader("forward_no_answer", row["cfwd_no_answer_number"])
   e:addHeader("forward_no_answer_enabled", tf_value(row["cfwd_no_answer_active"]))
   e:addHeader("foward_busy", row["cfwd_busy_number"])
   e:addHeader("forward_busy_enabled", tf_value(row["cfwd_busy_active"]))
   
   e:fire()
end


function action_callback( row )
   freeswitch.console_log("INFO", "action_callback\n")
   print_r(row)   
   
   local e = freeswitch.Event("SWITCH_EVENT_PHONE_FEATURE")
   e:addHeader("profile", row["profile_name"])
   e:addHeader("user", row["username"])
   e:addHeader("host", row["domain"])
   e:addHeader("device", row["username"])
   
   if row["event"] == "SetDoNotDisturb" then
      e:addHeader("Feature-Event", "DoNotDisturbEvent")
      e:addHeader("doNotDisturbOn", tf_value(row["dnd"]))
   elseif row["event"] == "SetCallForward" then
      e:addHeader("Feature-Event", "ForwardingEvent")
      if row["action_name"] == "forward_immediate" then
	 e:addHeader("forward_immediate_enabled", tf_value(row["cfwd_immediate_active"]))
	 e:addHeader("forward_immediate", row["cfwd_immediate_number"])
      elseif row["action_name"] == "forward_busy" then
	 e:addHeader("forward_busy_enabled", tf_value(row["cfwd_busy_active"]))
	 e:addHeader("forward_busy", row["cfwd_busy_number"])
      elseif row["action_name"] == "forward_no_answer" then
	 e:addHeader("forward_no_answer_enabled", tf_value(row["cfwd_no_answer_active"]))
	 e:addHeader("forward_no_answer", row["cfwd_no_answer_number"])
	 e:addHeader("ringCount", row["cfwd_no_answer_rings"])
      end
   end
   
   e:fire()
end


function print_r (t, indent)
   local indent=indent or ''
   for key,value in pairs(t) do
      freeswitch.console_clean_log(string.format("%s[%s]", indent, tostring(key)))
      if type(value)=="table" then 
       	 freeswitch.console_clean_log(":\n") 
	 print_r(value,indent..'\t')
      else 
    	 freeswitch.console_clean_log(string.format(" = %s\n", tostring(value))) 
      end
   end
end


freeswitch.console_log("INFO", "Hook: [Event-Name] = "..event:getHeader("Event-Name").."\n") 


local cseq     = event:getHeader('cseq')
local username = event:getHeader('user')
local domain   = event:getHeader('host')


dbh:query(string.format("SELECT COUNT(*) AS record_count FROM phone_features WHERE username = '%s' AND domain = '%s'", username, domain), count_callback)
freeswitch.console_log("INFO", string.format("we got %d records\n", record_count));

local feature_action  = event:getHeader("Feature-Action")
local feature_enabled = event:getHeader("Feature-Enabled")
local device          = event:getHeader("device")
local action_name     = event:getHeader("Action-Name")
local action_value    = event:getHeader("Action-Value")
local profile_name    = event:getHeader("profile_name")
local field_name      = nil

if feature_action == "SetDoNotDisturb" then
   enabled_field_name = "dnd"
elseif feature_action == "SetCallForward" then
   if action_name == "forward_immediate" then
      field_name = "cfwd_immediate_number"
      enabled_field_name = "cfwd_immediate_active"
   elseif action_name == "forward_no_answer" then
      field_name = "cfwd_no_answer_number"
      enabled_field_name = "cfwd_no_answer_active"
   elseif action_name == "forward_busy" then
      field_name = "cfwd_busy_number"
      enabled_field_name = "cfwd_busy_active"
   end
end

if not device then
   device = ''
end

if not action_value then
   action_value = ''
end

if record_count == "0" then
   local query = string.format("INSERT INTO phone_features (username, domain) VALUES ('%s', '%s');", username, domain)
   freeswitch.console_log("INFO", query .. "\n")
   dbh:query( query )
end

if field_name then
   local query
   if action_name == "forward_no_answer" then
      local ring_count = event:getHeader("ringCount")
      query = string.format("UPDATE phone_features SET %s = '%s', %s = '%s', cfwd_no_answer_rings = %d WHERE username = '%s' AND domain = '%s'", enabled_field_name, feature_enabled, field_name, action_value, ring_count, username, domain)
   else
      query = string.format("UPDATE phone_features SET %s = '%s', %s = '%s' WHERE username = '%s' AND domain = '%s'", enabled_field_name, feature_enabled, field_name, action_value, username, domain)
   end

   freeswitch.console_log("INFO", query .. "\n")
   dbh:query( query )
elseif enabled_field_name then
   local query = string.format("UPDATE phone_features SET %s = '%s' WHERE username = '%s' AND domain = '%s'", enabled_field_name, feature_enabled, username, domain)
   freeswitch.console_log("INFO", query .. "\n")
   dbh:query( query )
end

local query
if enabled_field_name then
   query = string.format("SELECT *, '%s' AS event, '%s' AS device, '%s' AS action_name, '%s' AS profile_name FROM phone_features WHERE username = '%s' AND domain = '%s';", 
			 feature_action, device, action_name, profile_name, username, domain)
   freeswitch.console_log("INFO", query .. "\n")
   dbh:query(query, action_callback)
else
   query = string.format("SELECT *, '%s' AS profile_name FROM phone_features WHERE username = '%s' AND domain = '%s';", profile_name, username, domain)
   freeswitch.console_log("INFO", query .. "\n")
   dbh:query(query, init_callback)
end


