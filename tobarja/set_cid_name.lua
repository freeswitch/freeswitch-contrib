require "split" -- I'm using the python-like example from http://lua-users.org/wiki/SplitJoin
require "trim" -- I'm using trim9 example from http://lua-users.org/wiki/StringTrim

local log = freeswitch.consoleLog

local caller_id = session:getVariable("caller_id_number")
local effective_caller_id = session:getVariable("effective_caller_id_number")
local CID

if effective_caller_id then
  log("debug",  "set_cid_name.lua: effective_caller_id: " .. effective_caller_id .. "\n");
  CID = effective_caller_id
else
  CID = caller_id
end

log("debug",  "set_cid_name.lua: caller_id: " .. caller_id .. "\n");
log("debug",  "set_cid_name.lua: CID: " .. CID .. "\n");

--My main provider prepends a '+' to inbound caller ids. My csv doesn't include these, so I snip it off.
if CID:sub(1,1) == "+" then
  CID = CID:sub(2)
  log("debug",  "set_cid_name.lua: removed plus:" .. CID .. "\n");
end

--Some US inbound providers don't send 1+10digits. My csv is 1+10digits, so I need this.
if CID:len() == 10 then
  CID = "1" .. CID
  log("debug",  "set_cid_name.lua: prepended '1':" .. CID .. "\n");
end

local file = io.open(argv[1])
if file then
  for line in file:lines() do
    local number, name = unpack(line:split(","))
    if CID == number then
      session:setVariable("effective_caller_id_name", trim(name));
      log("debug",  "set_cid_name.lua: match! " .. "\n");
      log("debug",  "set_cid_name.lua: name =<" .. trim(name) .. ">\n");
      log("debug",  "set_cid_name.lua: name:len() =" .. trim(name):len() .. "\n");
      log("info",  "set_cid_name.lua: match found: " .. CID .. " -> " .. trim(name) .. "\n");
      break
    end
  end
else
end
log("debug",  "set_cid_name.lua: script ended" .. "\n");
