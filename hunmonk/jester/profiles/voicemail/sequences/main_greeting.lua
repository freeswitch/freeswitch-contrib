greeting_keys = {
  ["#"] = ":break",
}

if profile.check_messages then
  greeting_keys["*"] = "login " .. profile.mailbox .. "," .. profile.domain
end

operator_on_record = ""
if profile.operator_extension then
  greeting_keys["0"] = "transfer_to_operator"
  operator_on_record = "operator"
end

return
{
  {
    action = "play_valid_file",
    files =  {
      profile.mailbox_dir .. "/temp.wav",
      profile.mailbox_dir .. "/unavail.wav",
      "phrase:default_greeting:" .. profile.mailbox,
    },
    keys = greeting_keys,
  },
  {
    action = "exit_sequence",
    sequence = "main_greeting_prepare_message",
  },
  {
    action = "call_sequence",
    sequence = "record_message " .. operator_on_record,
  },
}
