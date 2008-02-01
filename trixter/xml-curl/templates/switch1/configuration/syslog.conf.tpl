<?xml version="1.0"?>
<document type="freeswitch/xml">
  <section name="configuration" description="Various Configuration">
<configuration name="syslog.conf" description="Syslog Logger">
  <!-- SYSLOG -->
  <!-- emerg   - system is unusable  -->
  <!-- alert   - action must be taken immediately  -->
  <!-- crit    - critical conditions  -->
  <!-- err     - error conditions  -->
  <!-- warning - warning conditions  -->
  <!-- notice  - normal, but significant, condition  -->
  <!-- info    - informational message  -->
  <!-- debug   - debug-level message -->
  <settings>
    <param name="ident" value="freeswitch"/>
    <param name="facility" value="user"/>
    <param name="format" value="${time} - ${message}"/>
    <param name="level" value="debug,info,warning-alert"/>
  </settings>
</configuration>
</section>
</document>
