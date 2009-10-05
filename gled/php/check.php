#!/usr/bin/php -q
<?php
require_once('phpagi-2.14/phpagi.php');

$agi = new AGI();
logthis("New call incoming");

$agi->answer();

logthis("Answered");

$res = $agi->stream_file('/telephony/freeswitch/sounds/en/us/callie/ivr/8000/ivr-you_have_dialed_an_invalid_extension.wav',"*");

logthis("Stream_file res:".print_r($res,true));

$agi->hangup();

?>
