<?xml version="1.0"?>
<document type="freeswitch/xml">
  <section name="configuration" description="Various Configuration">
<!-- Valid fields in conditions: -->
<!-- "dialplan, caller_id_name, ani, ani2, caller_id_number, -->
<!-- rdnis, destination_number, uuid, source, context, chan_name" -->

<!-- *NOTE* The special context name 'any' will match any context -->
<context name="default">
  <extension name="556"> <!-- demo phrases -->
    <condition field="destination_number" expression="^556$">
      <action application="answer"/>
      <action application="sleep" data="1000"/>
      <action application="phrase" data="spell,${caller_id_name}"/>
      <action application="phrase" data="spell-phonetic,${caller_id_name}"/>
      <action application="phrase" data="timespec,12:45:15"/>
      <action application="phrase" data="saydate,0"/>
      <action application="phrase" data="msgcount,130"/>
      <action application="phrase" data="ip-addr,66.250.68.194"/>
      <action application="phrase" data="saydate,$strepoch(2006-03-23 7:23)"/>
      <!--<action application="phrase" data="timeleft,3:30"/>-->
    </condition>
  </extension>

  <extension name="9193">
    <condition field="destination_number" expression="^9193$">
      <action application="set" data="bridge_pre_execute_bleg_app=soundtouch"/>
      <!-- send or recv indicates which direction the dtmf is parsed from 
	   since this example is send and it's being called on the b leg
	   the application will intercept the dtmf from being sent to the b leg 
	   a.k.a. by the dtmf of the A leg.
	   if it were 'recv' then it would be parsed when the dtmf was 
	   received *from* the b leg so it could control itself.
	   The optional keywords "read" and "write" will also change the stream replaced
      -->
      <action application="set" data="bridge_pre_execute_bleg_data=send -4s"/>
      <action application="bridge" data="sofia/$${domain}/foo"/>
    </condition>
  </extension>



  <extension name="9192">
    <condition field="destination_number" expression="^9192$">
      <!-- Maintain Buffer of 128k of audio (default is 64k) -->
      <action application="set" data="stream_prebuffer=131072"/>
      <!-- Play a stream -->
      <action application="playback" data="shout://mp3.ihets.org/wfyihd132"/>
    </condition>
  </extension>


  <!-- Example extension for require auth per-call. -->
  <extension name="9191">
    <!-- Match the destination digits of 9191 -->
    <condition field="destination_number" expression="^9191$"/>
    <!-- Make sure the sip_authorized variable is set (set on all authed calls) 
	 If it isn't, then send an auth challange.
    -->
    <condition field="${sip_authorized}" expression="true">
      <anti-action application="reject" data="407"/>
    </condition>

    <!-- If you made it here all is well and the call is authed.
	 Do whatever you wish.
    -->
    <condition>
      <action application="playback" data="/tmp/itworked.wav"/>
    </condition>
  </extension>

  <extension name="tollfree">
    <condition field="destination_number" expression="^(18(0{2}|8{2}|7{2}|6{2})\d{7})$">
      <action application="enum" data="$1"/>
      <action application="bridge" data="${enum_auto_route}"/>
    </condition>
  </extension>

  <!-- Call the FreeSWITCH conference via SIP -->
  <!--<extension name="FreeSWITCH Conference SIP">-->
  <!--<condition field="destination_number" expression="^888$">-->
  <!--<action application="bridge" data="sofia/$${sip_profile}/888@conference.freeswitch.org"/>-->
  <!--</condition>-->
  <!--</extension> -->

  <!-- Call the FreeSWITCH conference via IAX -->
  <!--<extension name="FreeSWITCH Conference IAX">-->
  <!--<condition field="destination_number" expression="^8888$">-->
  <!--<action application="bridge" data="iax/guest@conference.freeswitch.org/888"/>-->
  <!--</condition>-->
  <!--</extension>-->

  <extension name="set_codec" continue="true">
    <condition field="source" expression="mod_portaudio">
      <action application="export" data="absolute_codec_string=$${global_codec_prefs}"/>
      <action application="export" data="nolocal:jitterbuffer_msec=180"/>
    </condition>
  </extension>


  <extension name="testmusic">
    <condition field="destination_number" expression="^1234$">
      <!-- Request a certain tone/file to be played while you wait for the call to be answered-->
      <action application="set" data="ringback=${us-ring}"/>
      <!--<action application="set" data="ringback=/home/ring.wav"/>-->
      <!--<action application="set" data="jitterbuffer_msec=180"/>-->
      <action application="bridge" data="sofia/$${sip_profile}/1234@conference.freeswitch.org"/>
    </condition>
  </extension>

  <!-- Enter an existing conference -->
  <extension name="1000">
    <condition field="destination_number" expression="^1000$">
      <action application="conference" data="freeswitch"/>
    </condition>
  </extension>

  <!-- Start a dynamic conference and call someone at the same time -->
  <extension name="2000">
    <condition field="destination_number" expression="^2000$">
      <action application="conference" data="bridge:mydynaconf:sofia/$${sip_profile}/1234@conference.freeswitch.org"/>
    </condition>
  </extension>

  <!-- extensions starting with 4, all the numbers after 4 form a numeric filename -->
  <!-- continue="true" means keep looking for more extensions to match -->
  <!-- *NOTE* The entire dialplan is parsed ONCE when the call starts -->
  <!-- so any call info acquired after the various actions cannot -->
  <!-- be taken into consideration. -->

  <!-- The first match will play a beep and the second one plays -->
  <!-- the desired file.  This is for demo purposes both actions -->
  <!-- could have been under the same <extension> tag as well. -->
  <extension name="playsound1" continue="true">
    <condition field="source" expression="mod_sofia"/>
    <condition field="destination_number" expression="^4(\d+)">
      <action application="playback" data="/var/sounds/beep.gsm"/>
    </condition>
  </extension>

  <extension name="playsound2">
    <condition field="source" expression="mod_sofia"/>
    <condition field="destination_number" expression="^4(\d+)">
      <action application="playback" data="/root/$1.raw"/>
    </condition>
  </extension>

  <!-- send everything with a certian RDNIS to Wanpipe ISDN -->
  <extension name="To PRI">
    <condition field="rdnis" expression="8881231234"/>
    <condition field="destination_number" expression="(.*)">
      <action application="bridge" data="wanpipe/pri/a/a/$1"/>
    </condition>
  </extension>

  <!-- Call *MUST* originate from mod_iax and also be dialing ext 9999-->
  <extension name="9999">
    <condition field="source" expression="mod_iax"/>
    <condition field="destination_number" expression="9999">
      <action application="playback" data="/var/sounds/beep.gsm"/>
    </condition>
  </extension>

  <!--This extension will start a conference and invite several people upon entering -->
  <extension name="0911">
    <condition field="destination_number" expression="0911">
      
      <!--These params effect the outcalls made once you join-->
      <action application="set" data="conference_auto_outcall_caller_id_name=pissed off boss"/>
      <action application="set" data="conference_auto_outcall_caller_id_number=0911"/>
      <action application="set" data="conference_auto_outcall_timeout=60"/>
      <action application="set" data="conference_auto_outcall_flags=none"/>
      <action application="set" data="conference_auto_outcall_announce=say:You have been called into an emergency conference"/>

      <!--Add as many of these as you need, These are the people you are going to call-->
      <action application="conference_set_auto_outcall" data="sofia/gateway/mygateway/12121231234"/>
      <action application="conference_set_auto_outcall" data="sofia/$${domain}/1234@somewhere.com"/>
      
      <action application="conference" data="cool@default"/>
    </condition>
  </extension>

</context>
</section>
</document>
