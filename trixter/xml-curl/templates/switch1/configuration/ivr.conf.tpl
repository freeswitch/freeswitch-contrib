<?xml version="1.0"?>
<document type="freeswitch/xml">
  <section name="configuration" description="Various Configuration">
<configuration name="ivr.conf" description="IVR menus">
  <menus>
    <menu name="main"
	  greet-long="/soundfiles/greet-long.wav" 
	  greet-short="/soundfiles/greet-short.wav"
	  invalid-sound="/soundfiles/invalid.wav"
	  exit-sound="/soundfiles/exit.wav" timeout ="15" max-failures="3">
      <entry action="menu-exit" digits="*"/>
      <entry action="menu-sub" digits="2" param="menu2"/>
      <entry action="menu-exec-api" digits="3" param="api arg"/>
      <entry action="menu-play-sound" digits="4" param="/soundfiles/4.wav"/>
      <entry action="menu-back" digits="5"/>
      <entry action="menu-call-transfer" digits="7" param="888"/>
      <entry action="menu-sub" digits="8" param="menu8"/>
    </menu>
    <menu name="menu8"
	  greet-long="/soundfiles/greet-long.wav"
	  greet-short="/soundfiles/greet-short.wav"
	  invalid-sound="/soundfiles/invalid.wav"
	  exit-sound="/soundfiles/exit.wav"
	  timeout ="15"
	  max-failures="3">
      <entry action="menu-back" digits="#"/>
      <entry action="menu-play-sound" digits="4" param="/soundfiles/4.wav"/>
      <entry action="menu-top" digits="*"/>
    </menu>
    <menu name="menu2"
	  greet-long="/soundfiles/greet-long.wav"
	  greet-short="/soundfiles/greet-short.wav"
	  invalid-sound="/soundfiles/invalid.wav"
	  exit-sound="/soundfiles/exit.wav"
	  timeout ="15"
	  max-failures="3">
      <entry action="menu-back" digits="#"/>
      <entry action="menu-play-sound" digits="4" param="/soundfiles/4.wav"/>
      <entry action="menu-top" digits="*"/>
    </menu>
  </menus>
</configuration> 
</section>
</document>
