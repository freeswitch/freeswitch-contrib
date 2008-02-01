<?xml version="1.0"?>
<document type="freeswitch/xml">
  <section name="configuration" description="Various Configuration">
<configuration name="xml_curl.conf" description="cURL XML Gateway">
  <bindings>
    <binding name="example">
      <!-- The url to a gateway cgi that can generate xml similar to
	   what's in this file only on-the-fly (leave it commented if you dont
	   need it) -->
      <!-- one or more |-delim of configuration|directory|dialplan -->
      <param name="gateway-url" value="http://www.mydomain.com/test.cgi" bindings="dialplan"/>
      <!-- set this to provide authentication credentials to the server -->
      <!--<param name="gateway-credentials" value="muser:mypass"/>-->
      <!-- set to true to disable Expect: 100-continue lighttpd requires this setting -->
      <!--<param name="disable-100-continue" value="true"/>-->
    </binding>
  </bindings>
</configuration>
</section>
</document>
