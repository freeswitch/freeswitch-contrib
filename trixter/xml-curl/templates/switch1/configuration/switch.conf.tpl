<?xml version="1.0"?>
<document type="freeswitch/xml">
  <section name="configuration" description="Various Configuration">
<configuration name="switch.conf" description="Modules">
  <settings>
    <!--Most channels to allow at once -->
    <param name="max-sessions" value="1000"/>
    <!--Try to catch any crashes that can be recoverable (in the context of a call)-->
    <param name="crash-protection" value="false"/>
    <!--RTP port range -->
    <!--<param name="rtp-start-port" value="16384"/>-->
    <!--<param name="rtp-end-port" value="32768"/>-->
  </settings>
  <!--Any variables defined here will be available in every channel, in the dialplan etc -->
  <variables>
    <variable name="uk-ring" value="%(400,200,400,450);%(400,2200,400,450)"/>
    <variable name="us-ring" value="%(2000, 4000, 440.0, 480.0)"/>
    <variable name="bong-ring" value="v=4000;>=0;+=2;#(60,0);v=2000;%(940,0,350,440)"/>
  </variables>
</configuration>

</section>
</document>
