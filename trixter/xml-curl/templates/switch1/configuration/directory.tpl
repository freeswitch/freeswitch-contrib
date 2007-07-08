<?xml version="1.0"?>
<document type="freeswitch/xml">
  <section name="configuration" description="Various Configuration">
<!--the domain or ip (the right hand side of the @ in the addr-->
<domain name="$${domain}">
  <!--the user id (the left hand side of the @ in the addr-->
  <user id="stpeter">
    <!-- Outbound Registrations Related to this user -->
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
    <params>
      <!-- omit password for authless registration -->
      <param name="password" value="mypass"/>
      <!-- optionally use this instead if you want to store the hash of user:domain:pass-->
      <!--<param name="a1-hash" value="c6440e5de50b403206989679159de89a"/>-->
    </params>
    <variables>
      <!--all variables here will be set on all inbound calls that originate from this user -->
      <variable name="testing" value="this"/>
      <!--<variable name="force-contact" value="nat-connectile-dysfunction"/>-->
    </variables>

    <vcard>
      <!-- insert optional compliant vcard xml here-->
    </vcard>
  </user>
</domain>
</section>
</document>
