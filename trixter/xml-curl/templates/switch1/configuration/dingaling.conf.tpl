<?xml version="1.0"?>
<document type="freeswitch/xml">
  <section name="configuration" description="Various Configuration">
<configuration name="dingaling.conf" description="XMPP Jingle Endpoint">
  <settings>
    <param name="debug" value="0"/>
    <param name="codec-prefs" value="PCMU"/>
  </settings>

  <!-- *NOTE* change <x-profile></x-profile> to <profile></profile> to enable -->

  <!-- Client Profile (Original mode) -->
  <x-profile type="client">
    <param name="name" value="$${xmpp_client_profile}"/>
    <param name="login" value="myjid@myserver.com/talk"/>
    <param name="password" value="mypass"/>
    <param name="dialplan" value="XML"/>
    <param name="message" value="Jingle all the way"/>
    <param name="rtp-ip" value="$${bind_server_ip}"/>
    <!-- <param name="ext-rtp-ip" value="$${external_rtp_ip"/> -->
    <param name="auto-login" value="true"/>
    <!-- SASL "plain" or "md5" -->
    <param name="sasl" value="plain"/>
    <!-- if the server where the jabber is hosted is not the same as the one in the jid -->
    <!--<param name="server" value="alternate.server.com"/>-->
    <!-- Enable TLS or not -->
    <param name="tls" value="true"/>
    <!-- disable to trade async for more calls -->
    <param name="use-rtp-timer" value="true"/>
    <!-- default extension (if one cannot be determined) -->
    <param name="exten" value="888"/>
    <!-- VAD choose one -->
    <!-- <param name="vad" value="in"/> -->
    <!-- <param name="vad" value="out"/> -->
    <param name="vad" value="both"/>
    <!--<param name="avatar" value="/path/to/tiny.jpg"/>-->
  </x-profile>

  <!-- Component (Server to Server Login) -->
  <x-profile type="component">
    <param name="name" value="$${xmpp_server_profile}"/>
    <param name="password" value="secret"/>
    <param name="dialplan" value="XML"/>
    <param name="rtp-ip" value="$${bind_server_ip}"/>
    <param name="server" value="jabber.server.org:5347"/>
    <!-- disable to trade async for more calls -->
    <param name="use-rtp-timer" value="true"/>
    <!-- "_auto_" means the extension will be automaticly set to the called jid -->
    <param name="exten" value="_auto_"/>
    <!--<param name="vad" value="both"/>-->
    <!--<param name="avatar" value="/path/to/tiny.jpg"/>-->
    <!--If you have ODBC support and a working dsn you can use it instead of SQLite-->
    <!--<param name="odbc-dsn" value="dsn:user:pass"/>-->

  </x-profile>

</configuration>
</section>
</document>
