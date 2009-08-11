<?php
declare(ticks=1);
pcntl_signal(SIGPIPE,  "sig_fs");

function sig_fs($signo) {
	file_put_contents('/devel/log/ivr.log',date("Y-m-d H:i:s").": BROKEN PIPE, call has been hung up\n",FILE_APPEND);
	exit();
}

class IVR {
	var $in = NULL;
	var $out = NULL;
	var $con_data = array();
	var $logfile = '/devel/log/ivr.log';

	function IVR() {
		ob_implicit_flush(true);
		$this->in = defined('STDIN') ? STDIN : fopen('php://stdin', 'r');
		$this->out = defined('STDOUT') ? STDOUT : fopen('php://stdout', 'w');

		stream_set_blocking( $this->in,0);

		fwrite($this->out,"connect\n\n");
		$buf = $this->wait_answer();
		if ($buf == '') {
			$this->log('Huhoo, cannot read setup infos... exiting');
			exit();
		}
		$this->con_data = $this->parse_response($buf);
		$this->log("New call uuid:{$this->con_data['channel-unique-id']}");
/*
		fwrite($this->out,"myevents\n\n");
		$buf = $this->wait_answer();
*/
	}

	function log($str) {
		file_put_contents($this->logfile,date("Y-m-d H:i:s").": $str\n",FILE_APPEND);
	}

	function check_hangup($ret='') {
		if ($ret == '')
			$ret = $this->parse_response($this->try_read());
		if ( isset( $ret['content-type'] ) && isset( $ret['content-disposition'] )
			&& $ret['content-type'] == 'text/disconnect-notice' 
			&& $ret['content-disposition'] == 'disconnect' )
				return true;
		return false;
	}

	function send_cmd($cmd,$app=FALSE,$args=FALSE) {
		fwrite($this->out, "sendmsg\n");
		fwrite($this->out, 'call-command:'. trim($cmd)."\n");
		if ($app)
			fwrite($this->out, 'execute-app-name: '.trim($app)."\n");
		if ($args)
			fwrite($this->out, 'execute-app-arg: '.trim($args)."\n\n");
		fwrite($this->out,"\n\n");
		return $this->wait_answer();
	}

	function execute($app,$args=FALSE) {
		return $this->send_cmd('execute',$app,$args);
	}

	function execute_wait($app,$args=FALSE) {
		return $this->send_cmd_wait('execute',$app,$args);
	}

	function send_cmd_wait($cmd,$app=FALSE,$args=FALSE) {
		fwrite($this->out, "sendmsg\n");
		fwrite($this->out, 'call-command:'. trim($cmd)."\n");
		if ($app)
			fwrite($this->out, 'execute-app-name: '.trim($app)."\n");
		if ($args)
			fwrite($this->out, 'execute-app-arg: '.trim($args)."\n\n");
		fwrite($this->out,"\n\n");
		return $this->wait_answer_block();
	}


	/*
	* Wait till we find an answer, trouble is that we may have a following and we cut.
	* we should sleep 100ms more to check if the response is closed ( or read till we find a \n\n
	*/
	function wait_answer() {
		$gc=0;
		$buf='';
		while ( $gc++ < 10000 ) { // we wait at max: 10 000 * 200 microseconds = 2 000 000 = 2seconds
			usleep(200);
			$c=0;
			do {
				$str = fgets($this->in);
				if ($str == "\n")
					break 2;
				else if ($str != '')
					$buf .= trim($str)."\n";
			} while (  $str != "" && $c++ < 100 );
		}
		return trim($buf);
	}

	function wait_answer_block() {
		$gc=0;
		$buf='';
		while ( true ) {
			usleep(200);
			$c=0;
			do {
				$str = fgets($this->in);
				if ($str == "\n")
					break 2;
				else if ($str != '')
					$buf .= trim($str)."\n";
			} while (  $str != "" && $c++ < 100 );
		}
		return trim($buf);
	}

	function try_read() {
		$c=0;
		$buf='';
		do {
			$str = fgets($this->in);
			if ($str == "\n")
				break;
			else if ($str != '')
				$buf .= trim($str)."\n";
		} while (  $str != "" && $c++ < 100 );
		return trim($buf);
	}

	/* Parse the returning values into an associative array */
	function parse_response($tmp) {
		$ret = array();
		if (trim($tmp) == '')
			return $ret;
		$tmp = explode("\n",$tmp);
		foreach ($tmp as $line) {
			list($key,$val) = explode(':',$line);
			$ret[strtolower($key)] = strtolower(trim( urldecode($val) ));
		}
		return $ret;
	}

	function check_retcode($ret) {
		if ( isset( $ret['content-type'] ) && isset( $ret['reply-text'] )
			&& $ret['content-type'] == 'command/reply' 
			&&  ( $ret['reply-text'] == '+ok' || $ret['reply-text'] == 'ok' ) )
				return true;
		return false;
	}

	function answer() {
		$ret = $this->parse_response($this->execute('answer'));
		if ( ! $this->check_retcode($ret) )
			$this->log("Huhooo Answer wasn't ack'd");
	}
	
	function ivr_sleep($time) {
		$ts = time() + $time;
		while ( time() < $ts ) {
			if ( $this->check_hangup() ) {
				$this->log('Call has been hungup, exiting...');
				exit();
			}
		}	
	}

	function hangup() {
		$ret = $this->parse_response($this->send_cmd('hangup'));
		if ( ! $this->check_retcode($ret) )
			$this->log("Huhooo Hangup wasn't ack'd");
	}

	/*
	* Play a file and returns when finished. ( wrong way to do the job, if user hangup we're not warned and wait for nothing )
	*/

	function playback($file) {
		$ret = $this->parse_response($this->execute_wait('playback',$file));
		if ( ! $this->check_retcode($ret) )
			$this->log("Huhooo playback wasn't ack'd");
	}


	function close() {
		if ($this->in != NULL)
			fclose($this->in);
		if ($this->out != NULL)
			fclose($this->out);
	}
}

?>
