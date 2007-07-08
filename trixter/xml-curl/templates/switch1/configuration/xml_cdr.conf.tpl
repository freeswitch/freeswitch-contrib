<?xml version="1.0"?>
<document type="freeswitch/xml">
  <section name="configuration" description="Various Configuration">
<configuration name="xml_cdr.conf" description="XML CDR CURL logger">
  <settings>
    <!-- the url to post to if blank web posting is disabled  -->
    <!-- <param name="url" value="http://localhost/cdr_curl/post.php"/> -->

    <!-- optional: credentials to send to web server -->
    <!--    <param name="cred" value="user:pass"/> -->

    <!-- the total number of retries (not counting the first 'try') to post to webserver incase of failure -->
    <!-- <param name="retries" value="2"/> -->

    <!-- delay between retries in seconds, default is 5 seconds -->
    <!-- <param name="retries" value="1"/> -->

    <!-- optional: full path to the error log dir for failed web posts if not specified its <freeswitch>/logs/xml_cdr_curl -->
    <!-- <param name="errLogDir" value="/tmp"/> -->

    <!-- optional: if not present we do not log every record to disk -->
    <!-- per original code base /xml_cdr is appended to the directory name -->
    <!-- defined but blank value will default to freeswitchdir/log -->
    <param name="logDir" value=""/>

  </settings>
</configuration>
</section>
</document>
