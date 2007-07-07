<?xml version="1.0"?>
<document type="freeswitch/xml">
  <section name="configuration" description="Various Configuration">
    <configuration name="wanpipe.conf" description="Sangoma Wanpipe Endpoint">
      <settings>
        <param name="debug" value="1"/>
        <param name="dialplan" value="XML"/>
        <param name="mtu" value="320"/>
        <param name="dtmf-on" value="800"/>
        <param name="dtmf-off" value="100"/>
        <param name="supress-dtmf-tone" value="yes"/>
      </settings>
      <span>
        <param name="span" value="1"/>
        <param name="node" value="cpe"/>
        <!-- <param name="switch" value="ni2"/> -->
        <param name="switch" value="dms100"/>
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
    </configuration>
  </section>
</document>
