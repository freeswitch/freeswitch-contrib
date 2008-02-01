<?xml version="1.0"?>
<document type="freeswitch/xml">
  <section name="configuration" description="Various Configuration">
<configuration name="console.conf" description="Console Logger">
  <!-- pick a file name, a function name or 'all' -->
  <!-- map as many as you need for specific debugging -->
  <mappings>
    <!-- <param name="log_event" value="DEBUG"/> -->
    <param name="all" value="DEBUG"/>
  </mappings>
  <settings>
    <!-- uncomment for color logging (for debugging) -->
    <!--<param name="colorize" value="true"/>-->
  </settings>
</configuration>
</section>
</document>
