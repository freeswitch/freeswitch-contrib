session:setAutoHangup(false)
session:answer();
session:set_tts_parms("flite", "kal");
session:speak("Welcome. Welcome to the VoIp World!. this is a Blind Users Community. powered by Freeswitch, the free / ultimate PBX. thank to anthm!");
session:sleep(100);
session:speak("please select an Action.");
session:sleep(100);
session:speak("to call the conference, press 1");
session:sleep(100);
session:speak("to call Freeswitch IVR, press 2");
session:sleep(100);
session:speak("to call Voice Mail, press 3");
session:sleep(100);
session:speak("for Music on hold, press 4");
session:sleep(100);
session:speak("to call me, press 0");
session:sleep(2000);
digits = session:getDigits(1, "", 3000);
if (digits == "1")  then
  session:execute("transfer","3001");
end
if (digits == "2")  then
  session:execute("transfer","5000");
end
if (digits == "3")  then
  session:execute("transfer","4000");
end
if (digits == "4")  then
  session:execute("transfer","9999");
end
if (digits == "0")  then
  session:execute("transfer","voipaware@sip.voipuser.org");
end