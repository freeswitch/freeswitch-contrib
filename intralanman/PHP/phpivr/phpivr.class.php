<?php

class phpivr {
	public $stdin;
	public $stderr;
	public $stdout;
	public $CHANNEL_DATA;

	private $hungup = false;

	public function __construct() {
		$this->stderr = fopen ( 'php://stderr', 'w' );
		$this->stdout = fopen ( 'php://stdout', 'w' );
		$this->stdin = fopen ( 'php://stdin', 'r' );
		$this->ivrd_connect ();
		$this->CHANNEL_DATA = $this->recv ();
		//$this->debug ( print_r ( $this->CHANNEL_DATA, true ) );
	}

	public function __destruct() {
		$this->debug ( "in function " . __FUNCTION__ );
		$this->ivrd_exit();
	}

	public function __call($method, $args) {
		$this->debug ( "in function " . __FUNCTION__ );
		$this->debug ( print_r ( $method, true ) );
		$this->debug ( print_r ( $args, true ) );
		$arg_string = '';
		$arg_count = count ( $args );
		for($i = 0; $i < $arg_count; $i ++) {
			if ($i == 0) {
				if (strstr ( $args[$i], ' ' )) {
					$arg_string = sprintf ( "'%s'", $args[$i] );
				} else {
					$arg_string = sprintf ( '%s', $args[$i] );
				}
			} else {
				if (strstr($args[$i], ' ') || empty($args[$i])) {
					$arg_string = sprintf ( "%s '%s'", $arg_string, $args[$i] );
				} else {
					$arg_string = sprintf ( '%s %s', $arg_string, $args[$i] );
				}
			}
		}
		if (! preg_match ( '/^(app|api|bgapi|config|directory|dialplan)_(.*)$/', $method, $matches )) {
			$this->debug ( "RegEx didn't match for[$method]" );
			return false;
		}
		$this->debug ( print_r ( $matches, true ) );
		$this->debug(sprintf("calling %s [%s] with args [%s]", $matches[1], $matches[2], $arg_string));
		switch ($matches[1]) {
			case 'app' :
				return $this->execute ( $matches[2], $arg_string );
					
			case 'api' :
				return $this->api ( $matches[2], $arg_string );

			case 'bgapi' :
				return $this->bgapi ( $matches[2], $arg_string );

			case 'config':
				return $this->fetch_config($matches[2], $arg_string);

			case 'directory':
				return $this->fetch_directory($matches[2], $arg_string);

			case 'dialplan':
				return $this->fetch_dialplan($matches[2], $arg_string);

			default:
				return false;
		}
	}

	public function ready() {
		if ($this->hungup) {
			return false;
		}
		return true;
	}

	public function ivrd_connect() {
		$this->debug ( "in function " . __FUNCTION__ );
		$this->send ( "connect" );
	}

	public function ivrd_exit() {
		$this->debug ( "in function " . __FUNCTION__ );
		$this->send ( "exit" );
	}

	public function debug($info) {
		fwrite ( $this->stderr, "$info\n" );
	}

	public function console_log($loglevel, $logdata) {
		$loglevel = strtoupper($loglevel);
		switch ($loglevel) {
			case 'ERROR':
				$loglevel = 'ERR';
				break;
			case 'WARN':
				$loglevel = 'WARNING';
				break;
			case 'CRITICAL':
				$loglevel = 'CRIT';
				break;
		}
		$this->execute('log', "$loglevel $logdata");
	}

	public function send($data) {
		if (!$this->stdout || !is_resource($this->stdout)) {
			return false;
		}
		$data .= "\n\n";
		fwrite ( $this->stdout, "$data" );
	}

	public function recv() {
		$event = new phpivr_event ();
		while ( $line = trim ( fgets ( $this->stdin ) ) ) {
			if (! strlen ( $line )) {
				break;
			}
			if (strstr ( $line, ':' )) {
				$line_parts = explode ( ':', $line );
				$key = trim ( array_shift ( $line_parts ) );
				$value = trim ( implode ( ':', $line_parts ) );
				$event->$key = $value;
			}
		}
		$content_len = $event->{'Content-Length'};
		if (! empty ( $content_len )) {
			$body = $this->recv_content_len ( $content_len );
			$event->set_body ( $body );
		}
		return $event;
	}

	public function recv_content_len($content_len) {
		//$this->debug ( "in function " . __FUNCTION__ );
		$recv_len = 0;
		$body = '';
		while ( $recv_len < $content_len ) {
			if ($content_len < 4096) {
				$len = ($content_len + 1);
			} else {
				$remainder = ($content_len - $recv_len);
				if ($remainder < 4096) {
					$len = ($remainder + 1);
				} else {
					$len = 4096;
				}
			}
			$body .= fgets ( $this->stdin, $len );
			$recv_len = strlen ( $body );
		}
		return $body;
	}

	public function send_msg($msg_data) {
		//$this->debug ( "in function " . __FUNCTION__ );
		$msg = sprintf ( "SendMsg %s\n%s\n", $this->CHANNEL_DATA->{"Unique-ID"}, $msg_data );
		$this->send ( $msg );
		return $this->recv ();
	}

	public function execute($app, $args) {
		//$this->debug ( "in function " . __FUNCTION__ );
		$msg = sprintf ( "call-command: execute\n" );
		$msg .= sprintf ( "execute-app-name: %s\n", $app );
		$msg .= sprintf ( "execute-app-arg: %s\n", $args );
		$this->send_msg ( $msg );
	}

	public function api($api, $args) {
		$api_cmd = sprintf ( 'api %s %s', $api, $args );
		$this->send ( $api_cmd );
		$event = $this->recv ();
		$body = $event->get_body ();
		return $body;
	}

	public function bgapi($api, $args) {
		$api_cmd = sprintf ( 'bgapi %s %s', $api, $args );
		$this->send ( $api_cmd );
		$event = $this->recv ();
		$body = $event->get_body ();
		return $body;
	}

	public function fetch_config($config) {
		$xml = $this->api_xml_locate('configuration', 'configuration', 'name', "$config.conf");
		return new SimpleXMLElement($xml);
	}

	public function fetch_dialplan() {
		$xml = $this->api_xml_locate();
		return new SimpleXMLElement($xml);
	}

	public function fetch_directory() {
		$xml = $this->api_xml_locate();
		return new SimpleXMLElement($xml);
	}

	public function fetch_user($ext, $domain) {
		$xml = $this->api_find_user_xml ( 'id', $ext, $domain );
		return new SimpleXMLElement ( $xml );
	}

	public function app_play_and_get_digits($min, $max, $tries, $time, $terminators, $file, $errfile, $var, $re) {
		$this->execute ( 'play_and_get_digits', "$min $max $tries $time $terminators $file $errfile $var $re" );
		$uuid = $this->CHANNEL_DATA->{"Unique-ID"};
		$this->debug ( "Var:[$var]" );
		$digits = $this->api_uuid_getvar ( $uuid, $var );
		return $digits;
	}

	public function var_undef($var) {
		if (empty($var)) {
			$this->debug('var is empty');
			return true;
		}
		if ($var == '_undef_') {
			$this->debug('var is _undef_');
			return true;
		}
	}
	
	public function app_hangup() {
		$this->hungup = true;
		$this->execute('app_hangup');
	}
}

class phpivr_event {
	private $stdin;
	private $stdout;
	private $stderr;
	private $headers;
	private $body;

	public function __construct() {
		$this->headers = new stdClass();
		$this->stderr = fopen ( 'php://stderr', 'w' );
		$this->stdout = fopen ( 'php://stdout', 'w' );
		$this->stdin = fopen ( 'php://stdin', 'r' );
	}

	public function __get($item) {
		$this->debug ( "in function " . __FUNCTION__ );
		if (! empty ( $this->headers->$item )) {
			return $this->headers->$item;
		}
	}

	public function __set($item, $value) {
		$this->debug ( "in function " . __FUNCTION__ );
		$value = trim ( rawurldecode ( $value ) );
		$this->debug ( "setting[$item] to[$value]" );
		$this->headers->$item = $value;
	}

	public function get_headers() {
		$this->debug ( "in function " . __FUNCTION__ );
		return $this->headers;
	}

	public function set_body($body) {
		$this->debug ( "in function " . __FUNCTION__ );
		$this->debug ( "Setting Body to[$body]" );
		$this->body = $body;
	}

	public function get_body() {
		$this->debug ( "in function " . __FUNCTION__ );
		$body = $this->body;
		return $body;
	}

	public function debug($info) {
		fwrite ( $this->stderr, "$info\n" );
	}
}
