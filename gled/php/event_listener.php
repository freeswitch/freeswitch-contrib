<?php

define ("PID","pid.event_listener.php");
require_once("processhandler.php");
require_once("ESL.php");

if ( ProcessHandler::isActive () )
        die();
else {
	/* Init stuff */
	ProcessHandler::activate ();

	$esl = new eslConnection('127.0.0.1', '8021', 'ClueCon');
	$esl->send('event PLAIN CHANNEL_HANGUP_COMPLETE');

	while ($esl->connected() && $e = $esl->recvEvent() ) {
		switch($e->getType()) {
			case 'CHANNEL_HANGUP_COMPLETE':
				$pid2kill = $e->getHeader('variable_pid2kill');
                                if ($pid2kill)
					posix_kill($pid2kill,SIGHUP);
			default:
				usleep(500);
			break;
		}
	}
}

?>
