return
{
  {
    action = "data_load",
    handler = "odbc",
    config = profile.db_config_messages,
    filters = {
      domain = storage("login_settings", "voicemail_domain"),
      mailbox = storage("login_settings", "mailbox_number"),
      __folder = args(1),
      __deleted = 0,
    },
    fields = {
      "__id",
      "caller_id_number",
      "caller_id_name",
      "caller_domain",
      "__timestamp",
      "__duration",
      "__deleted",
      "recording",
    },
    storage_area = "message" .. args(2),
    multiple = true,
    sort = "id",
  },
}

