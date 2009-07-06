<?php
include ("esl.php");
include ("esl_config.php");
echo ('<title>status</title>');
echo ('<p></p>');
 $fp = event_socket_create($host, $port, $password);
if ($_POST[mod]=='sofia'{
 $cmd = "api sofia status";
};
 $response = event_socket_request($fp, $cmd);
 echo $response; 
 fclose($fp);  
?>