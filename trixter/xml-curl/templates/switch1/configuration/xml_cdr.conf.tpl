<configuration name="xml_cdr.conf" description="XML CDR CURL logger">
  <settings>
    <!-- the url to post to if blank web posting is disabled  -->
    <!-- <param name="url" value="http://localhost/cdr_curl/post.php"/> -->

    <!-- optional: credentials to send to web server -->
    <!--    <param name="cred" value="user:pass"/> -->

    <!-- the total number of retries (not counting the first 'try') to post to webserver incase of failure -->
    <!-- <param name="retries" value="2"/> -->

    <!-- delay between retries in seconds, default is 5 seconds -->
    <!-- <param name="delay" value="1"/> -->

    <!-- optional: if not present we do not log every record to disk -->
    <!-- either an absolute path, a relative path assuming ${prefix}/logs or a blank value will default to ${prefix}/logs/xml_cdr -->
    <param name="log-dir" value=""/>
    
    <!-- encode the post data may be 'true' for url encoding, 'false' for no encoding or 'base64' for base64 encoding -->
    <!--<param name="encode" value="true"/>-->
    
    <!-- optional: full path to the error log dir for failed web posts if not specified its the same as log-dir -->
    <!-- either an absolute path, a relative path assuming ${prefix}/logs or a blank or omitted value will default to ${prefix}/logs/xml_cdr -->
    <!-- <param name="err-log-dir" value="/tmp"/> -->

    <!-- optional: if enabled this will disable CA root certificate checks by libcurl -->
    <!-- note: default value is disabled. only enable if you want this! -->
    <!-- <param name="ignore-cacert-check" value="true" /> -->

  </settings>
</configuration>
