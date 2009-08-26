--[[
  This script provides a simple mechanism to trim recordings made in
  FreeSWITCH.
  
  To use in the dialplan:
  Set the following channel variables:
  record_trim_file: Full path to the file you want trimmed.
  record_trim_pre:  Amount (in seconds) you want trimmed from the beginning
                    of the file.
  record_trim_post: Amount (in seconds) you want trimmed from the end of
                    the file.
  Then call this script.
  
  To use in another lua script:
  Make sure that the channel variable record_trim_file is not set.
  Include this script in your script.
  Call trim_file(file_to_trim, pre_trim_seconds, post_trim_seconds).
  
  Things to note:
  1. The trim values do not have to be integers.  You can do .5 seconds, for
     example.
  2. Currently the script only works with .wav files.

]]

function print_error(file_to_trim, error_message)
  freeswitch.consoleLog("error", string.format([[Error trimming file %s: %s]] .. "\n", file_to_trim, error_message))
end

function trim_file(file_to_trim, pre_trim_seconds, post_trim_seconds)
  -- Make sure the file is valid.
  local f = io.open(file_to_trim)
  if f then
    io.close(f)
    -- Test for the existence of sox.  Since sox seems to output it's help to
    -- stderr, and return 1 or 256 as the return code when it's installed,
    -- that's what we'll check for.  Weird, but functional.
    local result = os.execute("sox -h 2> /dev/null")
    if result == 1 or result == 256 then
      -- Get the length of the file in seconds from sox.  sox outputs the
      -- stat data to stderr, so bind stderr to stdout.
      local sox_info = assert(io.popen("sox " .. file_to_trim .. " -e stat 2>&1 | grep Length | awk '{print $3;}'", 'r'))
      local file_length = sox_info:read("*n")
      
      local trim_length
      if file_length then
        -- sox's trim command wants beginning position and length.
        -- Length is total length of the file minus both trim lengths.
        trim_length = file_length - pre_trim_seconds - post_trim_seconds
        if trim_length > 0 then
          -- Make a trimmed version of the file, then move it in
          -- place over the old version.
          result = os.execute(string.format([[sox %s %s.new.wav trim %f %f]], file_to_trim, file_to_trim, pre_trim_seconds, trim_length))
          if result == 0 then
            result = os.execute(string.format([[mv -f %s.new.wav %s]], file_to_trim, file_to_trim))
            if result == 0 then
              freeswitch.consoleLog("info", string.format([[Trimmed file %s at %f seconds, length %f seconds]] .. "\n", file_to_trim, pre_trim_seconds, trim_length))
            else
              print_error(file_to_trim, "unable to write trimmed file.")
            end
          else
            print_error(file_to_trim, "unable to run sox trim command.")
          end
        else
          print_error(file_to_trim, "negative trim length.")
        end
      else
        print_error(file_to_trim, "unable to find file length.")
      end
    else
      print_error(file_to_trim, "sox not installed.")
    end
  else
    print_error(file_to_trim, "unable to open file.")
  end
end

function check_for_file_to_trim()
  local file_to_trim = session:getVariable("record_trim_file")

  if file_to_trim then
    local pre_trim_seconds = session:getVariable("record_trim_pre")
    local post_trim_seconds = session:getVariable("record_trim_post")
    if not pre_trim_seconds then
      pre_trim_seconds = 0
    else
      pre_trim_seconds = tonumber(pre_trim_seconds)
    end
    if not post_trim_seconds then
      post_trim_seconds = 0
    else
      post_trim_seconds = tonumber(post_trim_seconds)
    end
    freeswitch.consoleLog("info", string.format([[Found file to trim: %s, %f seconds from beginning, %f seconds from end]] .. "\n", file_to_trim, pre_trim_seconds, post_trim_seconds))
    trim_file(file_to_trim, pre_trim_seconds, post_trim_seconds)
  end
end

if session and session:ready() then
  check_for_file_to_trim()
end
