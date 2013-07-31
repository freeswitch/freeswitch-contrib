
session:execute("start_dtmf", "")
session:execute("spandsp_start_dtmf", "")
session:execute("start_tone_detect", argv[1])
session:execute("park", "")

