#!/usr/bin/php -q
<?php
require_once('class.ivr.php');

$ivr = new IVR();
$ivr->answer();
$ivr->log('Answered call');
sleep(10);
$ivr->log('Calling ivr_sleep');
$ivr->ivr_sleep(50);
$ivr->log('Hanging up');
$ivr->hangup();
$ivr->close();

?>
