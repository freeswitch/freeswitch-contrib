<?xml version="1.0"?>
<document type="freeswitch/xml">
  <section name="configuration" description="Various Configuration">
<configuration name="alsa.conf" description="Soundcard Endpoint">
  <settings>
    <!--Default dialplan and caller-id info -->
    <param name="dialplan" value="XML"/>
    <param name="cid-name" value="N800 Alsa"/>
    <param name="cid-num" value="5555551212"/>

    <!--audio sample rate and interval -->
    <param name="sample-rate" value="8000"/>
    <param name="codec-ms" value="20"/>
  </settings>
</configuration>
</section>
</document>
