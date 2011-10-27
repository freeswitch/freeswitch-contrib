--[[
DISA (Direct Inward System Access)

I call this script this way:

        <action application="lua" data="disa.lua"/>
        <action application="set_user" data="1000@my.local.profile"/>
        <action application="transfer" data="${disa_dtn} XML default"/>
        <action application="hangup"/>

Change the default password!!!

This can be abused to grant anyone the ability to dial through your box. YOU HAVE BEEN WARNED.

You're going to need some sounds. Poke around the wiki, I think that's where I found the ones I use.

]]
session:answer()
session:setAutoHangup(false)

failcount = 0
pin = false

function getAUTH()
  local p
  p = session:playAndGetDigits(5, 10, 2, 7000, "#", "/opt/customsounds/8000/pin.wav", "", "\\d+")
  if p == '' then p = false end
  if (p == '24680' or p == '13579') then
    return p
  else
    session:streamFile("/opt/customsounds/8000/pin_incorrect.wav")
    return false
  end
end

function getDialToNumber()
  local d
  d = session:playAndGetDigits(4, 19, 2, 7000, "#", "/opt/customsounds/8000/please_enter_the_phone_number.wav", "", "\\d+")
  if d == '' then d = false end
  if d then
    return d
  else
    return false
  end
end

while failcount < 4 do repeat
  if (session:ready() == false) then
    failcount = 99
    break
  end
  session:sleep(700)
  if not pin then
    pin = getAUTH()
    if not pin then break end
  end

  if not dtn then
    dtn = getDialToNumber()
    if not dtn then break end
  end
  session:setVariable("disa_dtn", dtn)
  failcount = 9
  break
  
until true
freeswitch.consoleLog("info","failcount " .. failcount .. "\n");
failcount = failcount + 1
end
