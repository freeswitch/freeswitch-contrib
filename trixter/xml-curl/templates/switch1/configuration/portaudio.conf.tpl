<?xml version="1.0"?>
<document type="freeswitch/xml">
  <section name="configuration" description="Various Configuration">
<configuration name="portaudio.conf" description="Soundcard Endpoint">
  <settings>
    <!-- indev, outdev, ringdev: 
	 partial case sensitive string match on something in the name 
	 or the device number prefixed with # eg "#1" (or blank for default) -->

    <!-- device to use for input -->
    <param name="indev" value=""/>
    <!-- device to use for output -->
    <param name="outdev" value=""/>

    <!--device to use for inbound ring -->
    <!--<param name="ringdev" value=""/>-->
    <!--File to play as the ring sound -->
    <!--<param name="ring-file" value="/sounds/ring.wav"/>-->
    <!--Number of seconds to pause between rings -->
    <!--<param name="ring-interval" value="5"/>-->

    <!--file to play when calls are on hold-->
    <!--<param name="hold-file" value="/sounds/holdmusic.wav"/>-->
    <!--Timer to use for hold music (i'd leave this one commented)-->
    <!--<param name="timer-name" value="soft"/>-->

    <!--Default dialplan and caller-id info -->
    <param name="dialplan" value="XML"/>
    <param name="cid-name" value="FreeSwitch"/>
    <param name="cid-num" value="5555551212"/>

    <!--audio sample rate and interval -->
    <param name="sample-rate" value="8000"/>
    <param name="codec-ms" value="20"/>
  </settings>
</configuration>
</section>
</document>
