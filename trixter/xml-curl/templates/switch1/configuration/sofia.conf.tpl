{literal}
<?xml version="1.0"?>
<document type="freeswitch/xml">
  <section name="configuration" description="Various Configuration">
<configuration name="sofia.conf" description="sofia Endpoint">
  <profiles>
    <profile name="internal">
      <!--aliases are other names that will work as a valid profile name for this profile-->
      <aliases>
	<alias name="default"/>
      </aliases>
      <!-- Outbound Registrations -->
      <gateways>
	<!--<gateway name="asterlink.com">-->
	  <!--/// account username *required* ///-->
	  <!--<param name="username" value="cluecon"/>-->
	  <!--/// auth realm: *optional* same as gateway name, if blank ///-->
	  <!--<param name="realm" value="asterlink.com"/>-->
	  <!--/// domain to use in from: *optional* same as  realm, if blank ///-->
	  <!--<param name="from-domain" value="asterlink.com"/>-->
	  <!--/// account password *required* ///-->
	  <!--<param name="password" value="2007"/>--> 
	  <!--/// replace the INVITE from user with the channel's caller-id ///-->
	  <!--<param name="caller-id-in-from" value="false"/>-->
	  <!--/// extension for inbound calls: *optional* same as username, if blank ///-->
	  <!--<param name="extension" value="cluecon"/>-->
	  <!--/// proxy host: *optional* same as realm, if blank ///-->
	  <!--<param name="proxy" value="asterlink.com"/>-->
	  <!--/// expire in seconds: *optional* 3600, if blank ///-->
	  <!--<param name="expire-seconds" value="60"/>-->
	  <!--/// do not register ///-->
	  <!--<param name="register" value="false"/>-->
	  <!--How many seconds before a retry when a failure or timeout occurs -->
	  <!--<param name="retry_seconds" value="30"/>-->
	  <!--Use the callerid of an inbound call in the from field on outbound calls via this gateway -->
	  <!--<param name="caller-id-in-from" value="false"/>-->
	<!--</gateway>-->
      </gateways>

      <domains>
	<!-- indicator to parse the directory for domains with parse="true" to get gateways-->
	<!--<domain name="trxtel.com" parse="true"/>-->
      </domains>

      <settings>
	<param name="debug" value="1"/>
	<param name="rfc2833-pt" value="101"/>
	<param name="sip-port" value="5060"/>
	<param name="dialplan" value="XML"/>
	<param name="dtmf-duration" value="100"/>
	<param name="codec-prefs" value="PCMU@20i"/>
	<param name="codec-ms" value="20"/>
	<param name="use-rtp-timer" value="true"/>
	<param name="rtp-timer-name" value="soft"/>
	<param name="rtp-ip" value="0.0.0.0"/>
	<param name="sip-ip" value="0.0.0.0"/>
	<!--set to 'greedy' if you want your codec list to take precedence -->
	<param name="inbound-codec-negotiation" value="generous"/>
	<!-- if you want to send any special bind params of your own -->
	<!--<param name="bind-params" value="transport=udp"/>-->

	<!--If you don't want to pass through timestampes from 1 RTP call to another (on a per call basis with rtp_rewrite_timestamps chanvar)-->
	<!--<param name="rtp-rewrite-timestampes" value="true"/>-->

	<!--If you have ODBC support and a working dsn you can use it instead of SQLite-->
	<!--<param name="odbc-dsn" value="dsn:user:pass"/>-->

	<!--Uncomment to set all inbound calls to no media mode-->
	<!--<param name="inbound-no-media" value="true"/>-->

	<!--Uncomment to let calls hit the dialplan *before* you decide if the codec is ok-->
	<!--<param name="inbound-late-negotiation" value="true"/>-->

	<!-- this lets anything register -->
	<!--  comment the next line and uncomment one or both of the other 2 lines for call authentication -->
<!--	<param name="accept-blind-reg" value="true"/> -->

	<!--TTL for nonce in sip auth-->
	<param name="nonce-ttl" value="60"/>
	<!--Uncomment if you want to force the outbound leg of a bridge to only offer the codec 
	    that the originator is using-->
	<!--<param name="disable-transcoding" value="true"/>-->
	<!--<param name="auth-calls" value="true"/>-->
	<!-- on authed calls, authenticate *all* the packets not just invite -->
	<!--<param name="auth-all-packets" value="true"/>-->

	<!-- <param name="ext-rtp-ip" value="$${external_rtp_ip}"/>-->

	<!-- <param name="ext-sip-ip" value="100.101.102.103"/> -->
	<!-- VAD choose one (out is a good choice); -->
	<!-- <param name="vad" value="in"/> -->
	<!-- <param name="vad" value="out"/> -->
	<!-- <param name="vad" value="both"/> -->
	<!--<param name="alias" value="sip:10.0.1.251:5555"/>-->
      </settings>
    </profile>
  </profiles>
</configuration>
</section>
</document>
{/literal}
