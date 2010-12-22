module(..., package.seeall)

--[[
  Records a caller's message to a file.
]]
function record_file(action)
  local dir = action.location or "/tmp"
  local timestamp = os.time()
  local filename = action.filename
  local append = action.append and action.filename
  local pre_record_sound = action.pre_record_sound
  local pre_record_delay = action.pre_record_delay and tonumber(action.pre_record_delay) or 200
  local max_length = action.max_length and tonumber(action.max_length) or 180
  local silence_threshold = action.silence_threshold and tonumber(action.silence_threshold) or 20
  local silence_secs = action.silence_secs and tonumber(action.silence_secs) or 5
  local area = action.storage_area

  -- Default filename, formatted date and channel uuid.
  if not filename then
    filename = os.date("%Y-%m-%d_%H:%M:%S") .. "-" .. jester.channel.uuid .. ".wav"
  end
  local filepath = dir .. "/" .. filename
  jester.set_storage("record", "last_recording_name", filename)
  jester.set_storage("record", "last_recording_path", filepath)
  jester.set_storage("record", "last_recording_timestamp", timestamp)

  local record_append
  -- If the append param was set, activate record appending on the channel.
  -- Default filenames have the timestamp, so only set appending if a filename
  -- was specified.
  if append then
    -- Save the current state of the variable so it can be restored.
    record_append = jester.get_variable("record_append")
    jester.set_variable("record_append", "true")
  end

  if pre_record_sound then
    if pre_record_sound == "tone" then
      session:execute("playback", "tone_stream://%(1000, 0, 640)")
    else
      session:streamFile(pre_record_sound)
    end
  end

  if pre_record_delay > 0 then
    session:execute("sleep", pre_record_delay)
  end

  jester.debug_log("Recording file to location: %s", filepath)
  -- Capture recording duration by getting timestamps immediately before and
  -- after the recording.
  local startstamp = os.time()
  session:recordFile(filepath, max_length, silence_threshold, silence_secs)
  local endstamp = os.time() 

  if append then
    -- Restore the record_append variable.
    jester.set_variable("record_append", record_append)
  end

  jester.set_storage("record", "last_recording_duration", endstamp - startstamp)
  -- Store the recording settings in a custom set of keys if given.
  if area then
    jester.set_storage(area, "name", filename)
    jester.set_storage(area, "path", filepath)
    jester.set_storage(area, "timestamp", timestamp)
    jester.set_storage(area, "duration", endstamp - startstamp)
  end
end

