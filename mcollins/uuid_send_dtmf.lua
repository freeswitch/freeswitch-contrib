--[[ 

    uuid_send_dtmf.lua

Do uuid_send_dtmf from dialplan
accepts three args: uuid, time, and dtmf
uuid is the uuid of the session
time is the "+ <seconds>" value in seconds
dtmf is the sequence of dtmf digit(s) to send

NOTE: only the "+<seconds>" syntax of sched_api is supported here
NOTE: No error checking, so don't forget to send all args

Example dialplan call:
 <action application="lua" data="uuid_send_dtmf.lua ${uuid} 10 123"/>

--]]

-- Do not hangup at end of script
session:setAutoHangup(false);

-- get an api object
api = freeswitch.API();

-- get the calling args (uuid, time, dtmf)
uuid = argv[1];
time = argv[2];
dtmf = argv[3];

apicmd = "sched_api";
apiarg = "+" .. time .. " none uuid_send_dtmf " .. uuid .. " " .. dtmf;

freeswitch.console_log("NOTICE","apicmd: " .. apicmd .. "\n");
freeswitch.console_log("NOTICE","apiarg: " .. apiarg .. "\n\n");

if uuid then
    -- schedule the uuid_send_dtmf 
    res = api:execute(apicmd,apiarg);
    freeswitch.console_log("ERR","Result is " .. res .. "\n\n");
else
    freeswitch.console_log("ERR","Did not receive a uuid for this call!\n");
end

-- All done!
