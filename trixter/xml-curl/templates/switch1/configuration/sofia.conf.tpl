<?xml version="1.0"?>
<document type="freeswitch/xml">
  <section name="configuration" description="Various Configuration">
    <configuration name="sofia.conf" description="sofia Endpoint">
      <profiles>
        <profile name="mydomain1.com">
          <registrations>
            <!-- <registration name="asterlink">
                 <param name="register-scheme" value="Digest"/>
                 <param name="register-realm" value=""/>
                 <param name="register-username" value="1001"/>
                 <param name="register-password" value="nhy65tgb"/>
                 <param name="register-from" value="sip:1001@208.64.200.40"/>
                 <param name="register-to" value="sip:1001@conference.freeswitch.org"/>
                 <param name="register-proxy" value="sip:conference.freeswitch.org:5060"/>
                 <param name="register-frequency" value="20"/>
                 </registration> -->
          </registrations>
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
            <param name="rtp-ip" value="192.168.1.20"/>
            <param name="sip-ip" value="mydomain1.com"/>

            <!--Uncomment to set all inbound calls to no media mode-->
            <!--<param name="inbound-no-media" value="true"/>-->

            <!-- this lets anything register -->
            <!--  comment the next line and uncomment one or both of the other 2 lines for call authentication -->
            <param name="accept-blind-reg" value="true"/>

            <!--<param name="auth-calls" value="true"/>-->
            <!-- on authed calls, authenticate *all* the packets not just invite -->
            <!--<param name="auth-all-packets" value="true"/>-->

            <!-- optional ; -->
            <!-- <param name="ext-rtp-ip" value="stun:stun.server.com"/>-->
            <!-- <param name="ext-rtp-ip" value="100.101.102.103"/> -->
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
