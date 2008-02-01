<?xml version="1.0"?>
<document type="freeswitch/xml">
  <section name="configuration" description="Various Configuration">
<configuration name="wanpipe.conf" description="Sangoma Wanpipe Endpoint">
  <settings>
    <param name="debug" value="0"/>
    <param name="dialplan" value="XML"/>
    <param name="ms-per-frame" value="20"/>
    <param name="dtmf-on" value="800"/>
    <param name="dtmf-off" value="100"/>
    <param name="supress-dtmf-tone" value="yes"/>
  </settings>
  <ss7boost_handles>
<!--
    <handle>
      <param name="local-ip" value="127.0.0.65"/>
      <param name="local-port" value="53000"/>
      <param name="remote-ip" value="127.0.0.66"/>
      <param name="remote-port" value="53000"/>
    </handle>
-->
  </ss7boost_handles>
  <pri_spans>
    <!-- may be any single digit or a range of digits separated by a '-' e.g "1-4" -->
    <span id="1">
      <param name="node" value="cpe"/>
      <param name="switch" value="ni2"/>
      <!--<param name="switch" value="dms100"/>-->
      <!-- <param name="switch" value="lucent5e"/> -->
      <!-- <param name="switch" value="att4ess"/> -->
      <!-- <param name="switch" value="euroisdn"/> -->
      <!-- <param name="switch" value="gr303eoc"/> -->
      <!-- <param name="switch" value="gr303tmc"/> -->
      <param name="dp" value="national"/>
      <!-- <param name="dp" value="international"/> -->
      <!-- <param name="dp" value="local"/> -->
      <!-- <param name="dp" value="private"/> -->
      <!-- <param name="dp" value="unknown"/> -->
      <param name="l1" value="ulaw"/>
      <!-- <param name="l1" value="alaw"/> -->
      <param name="bchan" value="1-23"/>
      <param name="dchan" value="24"/>
      <param name="dialplan" value="XML"/>
    </span>
  </pri_spans>
</configuration>
</section>
</document>
