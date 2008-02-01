<?xml version="1.0"?>
<document type="freeswitch/xml">
  <section name="phrases" description="Speech Phrase Management">
    <macros>
      <language name="fr" sound_path="/var/sounds/lang/fr/jean" tts_engine="cepstral" tts_voice="jean-pierre">
<include><!--This line will be ignored it's here to validate the xml and is optional -->
<macro name="msgcount">
  <input pattern="(.*)">
    <match>
      <action function="play-file" data="tuas.wav"/>
      <action function="say" data="$1" method="pronounced" type="items"/>
      <action function="play-file" data="messages.wav"/>
    </match>
  </input>
</macro>
<macro name="timeleft">
  <input pattern="(\d+):(\d+)">
    <match>
      <action function="speak-text" data="il y a $1 minutes et de $2 secondes de restant"/>
    </match>
  </input>
</macro>
</include><!--This line will be ignored it's here to validate the xml and is optional -->
</language>
</macros>
</section>
</document>
