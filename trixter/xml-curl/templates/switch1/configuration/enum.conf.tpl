{literal}
<?xml version="1.0"?>
<document type="freeswitch/xml">
  <section name="configuration" description="Various Configuration">
<configuration name="enum.conf" description="ENUM Module">
  <settings>
    <param name="default-root" value="e164.org"/>
  </settings>

  <routes>
    <route service="E2U+SIP" regex="sip:(.*)" replace="sofia/$${sip_profile}/$1"/>
    <route service="E2U+IAX2" regex="iax2:(.*)" replace="iax/$1"/>
    <route service="E2U+XMPP" regex="XMPP:(.*)" replace="dingaling/$${xmpp_server_profile}/$1"/>
  </routes>
</configuration>
</section>
</document>
{/literal}
