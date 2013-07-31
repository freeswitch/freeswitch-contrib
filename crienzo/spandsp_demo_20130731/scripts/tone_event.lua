
local uuid = event:getHeader("Unique-ID")
if uuid and uuid ~= "" then
	local tone = event:getHeader("Detected-Tone")
	freeswitch.consoleLog("info", uuid .. " detected tone: " .. tone .. "\n")	
end

