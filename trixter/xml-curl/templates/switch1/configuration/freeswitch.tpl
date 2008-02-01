<?xml version="1.0"?>
<document type="freeswitch/xml">
  <section name="configuration" description="Various Configuration">
<?xml version="1.0"?>
<document type="freeswitch/xml">
  <!--#comment 
      All comments starting with #command will be preprocessed and never sent to the xml parser
      Valid instructions:
      #include ==> Include another file to this exact point
                   (partial xml should be encased in <include></include> tags)
      #set     ==> Set a global variable (can be expanded during preprocessing with $$ variables)
                   (note the double $$ which denotes preprocessor variables)
      #comment ==> A general comment such as this
      
      The preprocessor will compile the full xml document to ${prefix}/log/freeswitch.registry
      Don't modify it while freeswitch is running cos it is mem mapped in most cases =D
  -->

  <!-- Preprocessor Variables
       These are introduced when configuration strings must be consistent across modules. 
   -->
  <!-- sip_profile 
       Must be a domain name if you are being a registry server; otherwise
       can be any string.
       used by: sofia.conf.xml enum.conf.xml default_context.xml directory.xml
  --> 
  <!--#set "sip_profile=mydomain.com"-->
  <!-- xmpp_client_profile and xmpp_server_profile
       xmpp_client_profile can be any string. 
       xmpp_server_profile is appended to "dingaling_" to form the database name
       containing the "subscriptions" table.
       used by: dingaling.conf.xml enum.conf.xml 
  --> 
  <!--#set "global_codec_prefs=PCMU@20i"-->
  <!--#set "xmpp_client_profile=xmppc"-->
  <!--#set "xmpp_server_profile=xmpps"-->
  <!-- bind_server_ip
       Can be an ip address, a dns name, or "auto". 
       This determines an ip address available on this host to bind.
       If you are separating RTP and SIP traffic, you will want to have
       use different addresses where this variable appears.
       Used by: sofia.conf.xml dingaling.conf.xml
  -->
  <!--#set "bind_server_ip=auto"-->
  <!-- external_rtp_ip
       Used as the public IP address for SDP.
       Can be an ip address or a string like "stun:stun.server.com" 
       If unspecified, the bind_server_ip value is used.
       Used by: sofia.conf.xml dingaling.conf.xml
  -->
  <!--#set "external_rtp_ip=stun:stun.server.com"-->
  <!-- server_name
       A public ip address or DNS name that is used when advertising conference
       presence or registering sip.
       Used by: conference.conf.xml
  --> 
  <!-- outbound_caller_id and outbound_caller_name
       The caller ID telephone number we should use when calling out.
       Used by: conference.conf.xml
  -->
  <!--#set "outbound_caller_name=FreeSWITCH"-->
  <!--#set "outbound_caller_id=8777423583"-->

  <section name="configuration" description="Various Configuration">
    <!--#include "switch.conf.xml"-->
    <!--#include "modules.conf.xml"-->

    <!-- Order doesn't matter, but for clarity these are in same order as modules.conf.xml. 
         If they aren't loaded by modules.conf.xml, then they are ignored.
    --> 
    <!-- Loggers -->
    <!--#include "console.conf.xml"-->
    <!--#include "syslog.conf.xml"-->

    <!-- Multi-Faceted -->
    <!--#include "enum.conf.xml"-->

    <!-- XML Interfaces --> 
    <!--#include "xml_rpc.conf.xml"-->
    <!--#include "xml_cdr.conf.xml"-->
    <!--#include "xml_curl.conf.xml"-->
    <!-- none for mod_xml_cdr -->

    <!-- Event Handlers -->
    <!--#include "cdr.conf.xml"-->
    <!--#include "event_multicast.conf.xml"-->
    <!--#include "event_socket.conf.xml"-->
    <!--#include "xmpp_event.conf.xml"-->
    <!--#include "zeroconf.conf.xml"-->

    <!-- Directory Interfaces -->
    <!-- none for mod_ldap; dialplan_directory.conf.xml has ldap connection info -->

    <!-- Endpoints -->
    <!--#include "dingaling.conf.xml"-->
    <!--#include "iax.conf.xml"-->
    <!--#include "portaudio.conf.xml"-->
    <!--#include "alsa.conf.xml"-->
    <!--#include "sofia.conf.xml"-->
    <!--#include "wanpipe.conf.xml"-->
    <!--#include "woomera.conf.xml"-->

    <!-- Applications -->
    <!-- none for mod_bridgecall, mod_commands, mod_echo, mod_park, mod_playback --> 
    <!--#include "conference.conf.xml"-->
    <!-- ivr.conf is used by mod_dptools -->
    <!--#include "ivr.conf.xml"-->

    <!-- Dialplan Interfaces -->
    <!--#include "dialplan_directory.conf.xml"-->
    <!-- mod_dialplan_xml is configured in the separate "dialplan" section. -->

    <!-- Codec Interfaces -->
    <!-- no configuration needed -->
    <!-- File Format Interfaces -->
    <!-- no configuration needed -->
    <!-- Timers -->
    <!-- no configuration needed -->

    <!-- Languages -->
    <!--#include "spidermonkey.conf.xml"-->
    <!-- none for mod_perl -->

    <!-- ASR /TTS -->
    <!-- none for mod_cepstral -->
    <!--#include "rss.conf.xml"-->

    <!-- Say -->
    <!-- none for mod_say_en -->
    <!--#include "mod_cdr.conf.xml"-->
    <!--#include "mod_local_stream.conf.xml"-->
  </section>
  
  <section name="dialplan" description="Regex/XML Dialplan">
    <!--#include "default_context.xml"-->
  </section>

  <!-- mod_dingaling is reliant on the vcard data in the "directory" section. -->
  <!-- mod_sofia is reliant on the user data for authorization --> 
  <section name="directory" description="User Directory">
    <!--#include "directory.xml"-->
  </section>

  <!-- phrases section (under development still) -->
  <section name="phrases" description="Speech Phrase Management">
    <macros>
      <language name="en" sound_path="/snds" tts_engine="cepstral" tts_voice="david">
	<!--#include "lang_en.xml"-->
      </language>
      <language name="fr" sound_path="/var/sounds/lang/fr/jean" tts_engine="cepstral" tts_voice="jean-pierre">
	<!--#include "lang_fr.xml"-->
      </language>
    </macros>
  </section>

</document>


</section>
</document>
