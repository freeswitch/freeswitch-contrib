<?xml version="1.0"?>
<document type="freeswitch/xml">
  <section name="dialplan" description="Regex/XML Dialplan">
    <context name="default">
      <extension name="myextension">
        <condition field="destination_number" expression=".*">
          <action application="answer"/>
          <action application="playback" data="/sounds/oops.wav"/>
        </condition>
      </extension>
    </context>
  </section>
</document>
