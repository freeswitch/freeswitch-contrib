<?xml version="1.0"?>
<document type="freeswitch/xml">
  <section name="configuration" description="Various Configuration">
    <configuration name="iax.conf" description="IAX Configuration">
      <settings>
        <param name="debug" value="0"/>
        <!-- <param name="ip" value="1.2.3.4"> -->
        <param name="port" value="4569"/>
        <param name="dialplan" value="XML"/>
        <param name="codec-prefs" value="PCMU@20i,PCMA,speex,L16"/>
        <param name="codec-master" value="us"/>
        <param name="codec-rates" value="8"/>
      </settings>
    </configuration>
  </section>
</document>
