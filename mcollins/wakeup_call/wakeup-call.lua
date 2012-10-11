--
-- wakeup-call.lua
--
-- Simple wakeup call script
-- Reads caller id, checks for existing wakeup call
-- If wuc exists then confirm/delete, else do new wuc
--

-- Get the date library
require "date" 

api = freeswitch.API()
session:answer()
session:sleep(1000)
session:setAutoHangup(false)

-- Sound files and phrases
get_time  = "phrase:For_one_time_wakeup_call"
invalid   = "ivr/ivr-that_was_an_invalid_entry.wav"
already   = "phrase:Already_have_wakeup_call"
cancelled = "phrase:Wakeup_call_has_been_cancelled"
confirmed = "phrase:Wakeup_call_has_been_confirmed"

-- Dates/times/fun stuff
now_epoch = api:execute("strepoch")
now_iso   = api:execute("strftime")
today     = api:execute("strftime","%Y-%m-%d")
today_obj = date(today)
tom_obj   = today_obj:adddays(1)
tomorrow  = tom_obj:fmt("%Y-%m-%d")
freeswitch.consoleLog("INFO","Today is: " .. today .. "\n")
freeswitch.consoleLog("INFO","Tomorrow is: " .. tomorrow .. "\n")


--freeswitch.consoleLog("INFO","OS Time is " .. osdate .. "\n")

-- Check for existing wuc
caller = session:getVariable("caller_id_number")
my_wuc = api:execute("db","select/wuc/" .. caller)

while (session:ready()) do
	if ( my_wuc == '' ) then
		wakeup_dialstring = "originate {ignore_early_media=true,origination_caller_id_number=9253,origination_caller_id_name='Wakeup Call'}user/" .. caller .. " wake_up_sleepy_head"
		freeswitch.consoleLog("INFO","Setting wakeup dialstring to:\n" .. wakeup_dialstring .. "\n\n")
		-- No wuc, ask caller to create one
		session:streamFile("ivr/ivr-not_requested_wakeup_call.wav");
		freeswitch.consoleLog("INFO", "No wakeup call for " .. caller .. ", let's create one\n")
		digits = session:playAndGetDigits(4,4,3,9000,"#",get_time,invalid,"([012][0-9]|2[0123])[012345][0-9]")

		if ( session:ready() ) then
			-- Caller entered a valid time, let's proceed
			freeswitch.consoleLog("WARNING", "Digits entered: " .. digits .. "\n\n")
			-- digits has the military time HHMM
			-- Compare to time now to decide if we need today or tomorrow
			-- Calculate now in minutes since 00:00
			now_hr  = api:execute("strftime","%H")
			now_min = api:execute("strftime","%M")
		    	now_mod = (now_hr * 60) + now_min
			freeswitch.consoleLog("INFO","Now hr/min/mod [" .. now_hr .. "/" ..  now_min .. "/" .. now_mod .. "]\n")
	
			-- Calculate wuc time in minutes since midnight
			wuc_hr  = string.sub(digits, 1, 2)
		    	wuc_min = string.sub(digits, 3)
			freeswitch.consoleLog("INFO","wuc_hr: " .. wuc_hr .."  wuc_min: " .. wuc_min .. "\n")
			wuc_mod = (wuc_hr * 60) + wuc_min
			freeswitch.consoleLog("INFO","Wuc hr min mod [" .. wuc_hr .. "/" .. wuc_min .. "/" .. wuc_mod .. "]\n")

			-- Compare now vs. wuc time and act accordingly
			if ( wuc_mod < now_mod ) then
				-- Tomorrow
				-- Create date/time string
				wuc_time = tomorrow .. " " .. wuc_hr .. ":" .. wuc_min .. ":00"
				wuc_epoch = api:execute("strepoch",wuc_time)
				freeswitch.consoleLog("INFO","Setting wuc for tomorrow: " .. wuc_time .. " [" .. wuc_epoch .. "]\n")
				freeswitch.consoleLog("INFO", "Set sched_api for tomorrow...\n")
			else
				-- Later today
				wuc_time = today .. " " .. wuc_hr .. ":" .. wuc_min .. ":00"
				wuc_epoch = api:execute("strepoch",wuc_time)
				-- TODO: ask user if they want the wuc tomorrow instead of today
				freeswitch.consoleLog("INFO","Setting wuc for today: " .. wuc_time .. " [" .. wuc_epoch .. "]\n")
				freeswitch.consoleLog("INFO", "Set sched_api for today...\n")
			end
	
			sched_cmd = wuc_epoch .. " wuc" .. caller .. " " .. wakeup_dialstring
			freeswitch.consoleLog("ERR","sched_api args:\n" .. sched_cmd .. "\n")
			res = api:execute("sched_api",sched_cmd)
			freeswitch.consoleLog("INFO","sched_api result: " .. res)
			res = api:execute("sched_api",wuc_epoch .. " wuc" .. caller .. " db delete/wuc/"  .. caller)
			freeswitch.consoleLog("INFO","sched_api result: " .. res)
			res = api:execute("db","insert/wuc/" .. caller .. "/" .. wuc_epoch)
			freeswitch.consoleLog("INFO","db insert result: " .. res)

			-- Confirm the wakeup call
			my_confirm = confirmed .. ":" .. wuc_epoch
			session:execute("playback",my_confirm)

		else
			-- Caller hung up without entering anyting
			freeswitch.consoleLog("WARNING","Caller hung up without entering a wakeup time\n")
			session:hangup()
		end

	else
		-- Caller already has wuc, ask to cancel
		freeswitch.consoleLog("INFO", caller .. " has a wakeup call, ask to confirm or delete...\n")
		wuc_epoch = api:execute("db","select/wuc/" .. caller)
		freeswitch.consoleLog("INFO","wuc time for " .. caller .. " is " .. wuc_epoch .. "\n")
		already = already .. ":" .. wuc_epoch
		freeswitch.consoleLog("INFO","Calling playAndGetDigits with " .. already .. "\n")
		digits = session:playAndGetDigits(1,1,3,9000,"#",already .. ":" .. wuc_epoch,invalid,"[0-9*#]")

		if ( digits ~= '1' ) then
			api:execute("sched_del","wuc" .. caller)
			api:execute("db","delete/wuc/" .. caller)
			session:execute("playback", cancelled)
		else
			session:execute("playback", confirmed)
		end

	end -- if (my_wuc == '')
	session:hangup()

end -- while(session:ready())
session:hangup()
