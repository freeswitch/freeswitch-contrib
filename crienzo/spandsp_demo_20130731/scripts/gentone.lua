
local tones = {}
tones["us-ring"] = "%(2000,4000,440,480)"
tones["us-reorder"] = "%(300,200,480,620)"
tones["us-busy"] = "%(500,500,480,620)"
tones["uk-ring"] = "%(400,200,400,450);%(400,2000,400,450)"
tones["uk-reorder"] = "%(400,350,400);%(225,525,400)"
tones["uk-busy"] = "%(375,375,400)"
tones["de-ring"] = "%(1000,4000,425)"
tones["de-reorder"] = "%(240,240,425)"
tones["de-busy"] = "%(480,480,425)"
tones["dtmf-1-long"] = "%(1000,200,697,1209)"
tones["dtmf-1-huge"] = "%(4000,200,697,1209)"
tones["dtmf-1-short"] = "%(60,200,697,1209)"

local tone_name = argv[1]

local tone = tones[tone_name]
if not tone then
	tone = tone_name
end
if tone and tone ~= "" then
	session:preAnswer()
	session:streamFile("tone_stream://" .. tone .. ";loops=" .. argv[2])
end

