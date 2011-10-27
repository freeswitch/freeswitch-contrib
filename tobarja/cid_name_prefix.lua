--[[
cid_name_prefix.lua: Adds the specified prefix to the effective_caller_id of inbound calls.

I call this script this way:

        <action application="lua" data="cid_name_prefix.lua XX"/>

This will pick up any already set caller_id_name and prepend "XX " (x, x, space).

Useful when multiple DIDs or departments ring to your phone.

]]
require "split" -- I'm using the python-like example from http://lua-users.org/wiki/SplitJoin
require "trim" -- I'm using trim9 example from http://lua-users.org/wiki/StringTrim

local log = freeswitch.consoleLog

local caller_id_name = session:getVariable("caller_id_number_name")
local effective_caller_id_name = session:getVariable("effective_caller_id_name")
local CIDNAME

if effective_caller_id_name then
  log("debug",  "cid_name_prefix.lua: effective_caller_id_name: " .. effective_caller_id_name .. "\n");
  CIDNAME = effective_caller_id_name
else
  CIDNAME = caller_id_name
end

if CIDNAME == nil then
  CIDNAME = session:getVariable("effective_caller_id_number")
end
if CIDNAME == nil then
  CIDNAME = session:getVariable("caller_id_number")
end
if CIDNAME == nil then
  CIDNAME = ''
end

--log("debug",  "cid_name_prefix.lua: caller_id_name: " .. caller_id_name .. "\n");
log("debug",  "cid_name_prefix.lua: CIDNAME: " .. CIDNAME .. "\n");

if argv[1] then
  prefix = argv[1] .. ' '
else
  prefix = nil
end

if prefix then
  log("debug",  "set_cid_name.lua: prefix: " .. prefix .. "\n");
  session:setVariable("effective_caller_id_name", prefix .. CIDNAME);
  log("info",  "set_cid_name.lua: effective_caller_id_name: " .. prefix .. CIDNAME .."\n");
end

log("debug",  "cid_name_prefix.lua: script ended" .. "\n");
