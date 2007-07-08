<?xml version="1.0"?>
<document type="freeswitch/xml">
  <section name="configuration" description="Various Configuration">
<configuration name="post_load_modules.conf" description="Modules">
  <modules>
    <!-- Multi-Faceted -->
    <!-- mod_enum is a dialplan interface, an application interface and an api command interface -->
    <load module="mod_enum"/>

    <!-- XML Interfaces -->
    <load module="mod_xml_rpc"/>
    <load module="mod_xml_cdr"/>

    <!-- Event Handlers -->
    <!-- <load module="mod_cdr"/> -->
    <!-- <load module="mod_event_multicast"/> -->
    <load module="mod_event_socket"/>
    <!-- <load module="mod_xmpp_event"/> -->
    <!-- <load module="mod_zeroconf"/> -->

    <!-- Directory Interfaces -->
    <!-- <load module="mod_ldap"/> -->

    <!-- Endpoints -->
    <!-- <load module="mod_dingaling"/> -->
    <!--<load module="mod_iax"/>-->
    <load module="mod_portaudio"/>
    <load module="mod_alsa"/>
    <load module="mod_sofia"/>
    <!-- <load module="mod_wanpipe"/> -->
    <!-- <load module="mod_woomera"/> -->

    <!-- Applications -->
    <load module="mod_commands"/>
    <load module="mod_conference"/>
    <load module="mod_dptools"/>

    <!-- Dialplan Interfaces -->
    <!-- <load module="mod_dialplan_directory"/> -->
    <load module="mod_dialplan_xml"/>

    <!-- Codec Interfaces -->
    <load module="mod_g711"/>
    <load module="mod_gsm"/>
    <!-- <load module="mod_ilbc"/> -->
    <load module="mod_l16"/>
    <!-- <load module="mod_speex"/> -->

    <!-- File Format Interfaces -->
    <load module="mod_sndfile"/>
    <load module="mod_native_file"/>
    <!--For icecast/mp3 streams/files-->
    <!--<load module="mod_shout"/>-->
    <!--For local streams (play all the files in a directory)-->
    <!--<load module="mod_local_stream"/>-->

    <!-- Timers -->
    <load module="mod_softtimer"/>

    <!-- Languages -->
    <load module="mod_spidermonkey"/>
    <!-- <load module="mod_perl"/> -->

    <!-- ASR /TTS -->
    <load module="mod_cepstral"/>
    <!-- <load module="mod_rss"/> -->
    
    <!-- Say -->
    <load module="mod_say_en"/>
  </modules>
</configuration>
</section>
</document>
