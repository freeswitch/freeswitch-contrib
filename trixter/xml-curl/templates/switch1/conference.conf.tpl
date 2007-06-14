<?xml version="1.0"?>
<document type="freeswitch/xml">
  <section name="configuration" description="Various Configuration">
    <configuration name="conference.conf" description="Audio Conference">
      <!-- Profiles are collections of settings you can reference by name. -->
      <profiles>
        <profile name="default">
          <!-- Domain (for presence) -->
          <param name="domain" value="freeswitch.org"/>
          <!-- Sample Rate-->
          <param name="rate" value="8000"/>
          <!-- Number of milliseconds per frame -->
          <param name="interval" value="20"/>

          <!-- TTS Engine to use -->
          <param name="tts-engine" value="cepstral"/>
          <!-- TTS Voice to use -->
          <param name="tts-voice" value="david"/>

          <!-- If TTS is enabled all audio-file params not beginning with -->
          <!-- '/' or with drive: (i.e. c:) will be considered text to say with TTS -->

          <!-- File to play to acknowledge succees -->
          <param name="ack-sound" value="/sounds/{$CONFNAME}/beep.raw"/>
          <!-- File to play to acknowledge failure -->
          <param name="nack-sound" value="/sounds/{$CONFNAME}/beeperr.raw"/>
          <!-- File to play to acknowledge muted -->
          <param name="muted-sound" value="/sounds/{$CONFNAME}/conf-muted.raw"/>
          <!-- File to play to acknowledge unmuted -->
          <param name="unmuted-sound" value="/sounds/{$CONFNAME}/conf-unmuted.raw"/>
          <!-- File to play if you are alone in the conference -->
          <param name="alone-sound" value="/sounds/{$CONFNAME}/conf-onlyperson.raw"/>
          <!-- File to play when you join the conference -->
          <param name="enter-sound" value="/sounds/{$CONFNAME}/conf-enter.raw"/>-->
          <!-- File to play when you leave the conference -->
          <param name="exit-sound" value="/sounds/{$CONFNAME}/conf-exit.raw"/>-->
          <!-- File to play when you ae ejected from the conference -->
          <param name="kicked-sound" value="/sounds/{$CONFNAME}/conf-kicked.raw"/>
          <!-- File to play when the conference is locked -->
          <param name="locked-sound" value="/sounds/{$CONFNAME}/conf-locked.raw"/>
          <!-- File to play to prompt for a pin -->
          <param name="pin-sound" value="/sounds/{$CONFNAME}/conf-getpin.raw"/>
          <!-- File to play to when the pin is invalid -->
          <param name="bad-pin-sound" value="/sounds/{$CONFNAME}/conf-invalidpin.raw"/>
	  <param name="max-members-sound" value="/sounds/{$CONFNAME}/conf-full.raw"/>



{foreach key=k item=v from=$params}
          <param name="{$k}" value="{$v}"/>
{/foreach}

          <!-- Default Caller ID Name for outbound calls -->
          <param name="caller-id-name" value="FreeSWITCH"/>
          <!-- Default Caller ID Number for outbound calls -->
          <param name="caller-id-number" value="1234567890"/>
          <param name="caller-controls" value="mykeys"/>
        </profile>
      </profiles>
      <caller-controls>
        <group name="mykeys">
{foreach key=k item=v from=$digits}
          <control action="{$v}" digits="{$k}"/>
{/foreach}
        </group>
      </caller-controls>

    </configuration>
  </section>
</document>
