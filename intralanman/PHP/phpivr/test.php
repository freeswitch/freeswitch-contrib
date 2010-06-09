#!/usr/bin/php
<?php
require_once('phpivr.class.php');
class IVR_Voicemail extends phpivr {

	public $ext;
	public $user;
	public $domain;
	public $mailbox;
	public $voicemail_conf;
	public $voicemail_params;
	public $voicemail_context = 'default';

	public $email = 0;
	public $msgs = array ('new' => array(), 'saved' => array());

	public function __destruct() {
		$this->app_playback('phrase:voicemail_goodbye');
		$this->app_hangup();
		parent::__destruct();
	}

	function vm_auth() {
		$phrase = sprintf('phrase:voicemail_enter_id:%s'
		, $this->voicemail_params[$this->voicemail_context]['terminator-key']
		);
		$ext = $this->app_play_and_get_digits ( 3, 15
		, $this->voicemail_params[$this->voicemail_context]['max-retries'], 2000
		, $this->voicemail_params[$this->voicemail_context]['terminator-key'], $phrase
		, 'phrase:voicemail_invalid_extension', 'MAILBOX', '\\\d+'
		);
		$this->debug ( "Extension:[$ext]" );
		if ($this->var_undef($ext)) {
			$this->app_playback('phrase:voicemail_invalid_extension');
			return false;
		}
		$domain = $this->CHANNEL_DATA->{"FreeSWITCH-IPv4"};
		$this->debug ( "Host IP:[$domain]" );
		$sxml = $this->fetch_user($ext, $domain);
		$this->user = $sxml;
		if (!$sxml) {
			$this->app_playback('phrase:voicemail_invalid_extension');
			return false;
		}
		$phrase = sprintf('phrase:voicemail_enter_pass:%s'
		, $this->voicemail_params[$this->voicemail_context]['terminator-key']
		);
		$pass = $this->app_play_and_get_digits ( 4, 32
		, $this->voicemail_params[$this->voicemail_context]['max-retries'], 2000
		, $this->voicemail_params[$this->voicemail_context]['terminator-key'], $phrase
		, 'voicemail/vm-that_was_an_invalid_ext.wav', 'PASS', '\\\d+'
		);
		if ($this->var_undef($pass)) {
			$this->debug("VM Auth FAILED");
			$this->app_playback('phrase:voicemail_fail_auth');
			return false;
		}

		$xml = $this->api_find_user_xml ( 'id', $ext, $domain );
		$sxml = new SimpleXMLElement ( $xml );

		$this->debug("Attempted Password: [$pass]");
		foreach ( $sxml->params->param as $param ) {
			$this->debug ( "Name: " . $param['name'] );

			if ($param['name'] == 'vm-password') {
				$this->debug ( 'found vm-password' );
				if ($param['value'] == $pass) {
					$this->ext = $ext;
					$this->domain = $domain;
					return true;
				} else {
					$this->debug("VM Auth FAILED");
					$this->console_log("ERR", "Invalid Password for $ext@$domain");
					$this->app_playback('phrase:voicemail_fail_auth');
					return false;
				}
			}
		}
		foreach ($sxml->variables->variable as $var) {
			$this->debug("Name: " . $var['name']);
			if (preg_match('/^(email_addr|vm-mailto)$/', $var['value'])) {
				$this->email++;
			}
		}
	}

	public function get_messages() {
		$msgs = $this->api_vm_list( $this->mailbox );
		if (preg_match('/^-ERR/', $msgs)) {
			return false;
		}
		$this->debug( $msgs );
		$msg_array = explode ( "\n", $msgs );
		foreach ( $msg_array as $msg ) {
			$msg_array = explode( ":", $msg );
			$msg_obj = new stdClass();
			$msg_obj->timestamp = $msg_array[0];
			$msg_obj->folder = $msg_array[4];
			$msg_obj->file_path = $msg_array[5];
			$msg_obj->uuid = $msg_array[6];
			$msg_obj->cid_name = $msg_array[7];
			$msg_obj->cid_num = $msg_array[8];
			if ($msg_obj->folder == 'inbox') {
				array_push( $this->msgs['new'],  $msg_obj);
			} else {
				array_push( $this->msgs['saved'],  $msg_obj);
			}
		}
	}

	public function play_messages($folder) {
		$this->debug("Folder [$folder]");
		$this->debug ( print_r ( $this->msgs[$folder], true ) );
		$msg_count = count ( $this->msgs[$folder] );
		for($i = 0; $i < $msg_count; $i ++) {
			$this->debug ( print_r ( $this->msgs[$folder][$i], true ) );
			$this->play_message_metadata($this->msgs[$folder][$i], $i+1, $folder);
			$this->play_message ( $this->msgs[$folder][$i] );
		}
	}

	public function play_message_metadata($msg, $msg_number, $folder) {
		// play message number
		$phrase = sprintf('phrase:voicemail_say_message_number:%s:%d', $folder, $msg_number);
		$this->app_playback($phrase);

		// play phone number
		if (intval($msg->cid_num)) {
			$phrase = sprintf('phrase:voicemail_say_phone_number:%s', $msg->cid_num);
			$this->app_playback($phrase);
		}

		// play date
		$phrase = sprintf('phrase:voicemail_say_date:%s', $msg->timestamp);
		$this->app_playback($phrase);
	}

	public function play_message($msg) {
		$this->debug(print_r($msg, true));
		$digit = $this->app_play_and_get_digits ( 1, 1, 1, 0
		, $this->voicemail_params[$this->voicemail_context]['terminator-key'], $msg->file_path
		, '_undef_', 'TMP', '\\\d+'
		);
		if (!$this->var_undef($digit)) {
			$this->debug("DIGIT IS: [$digit]");
			goto digit_choice;
		}
		$phrase = sprintf('phrase:voicemail_listen_file_check:%s:%s:%s:%s:%s:%s%s'
		, $this->voicemail_params[$this->voicemail_context]['listen-file-key']
		, $this->voicemail_params[$this->voicemail_context]['save-file-key']
		, $this->voicemail_params[$this->voicemail_context]['delete-file-key']
		, $this->voicemail_params[$this->voicemail_context]['email-key']
		, $this->voicemail_params[$this->voicemail_context]['callback-key']
		, $this->voicemail_params[$this->voicemail_context]['forward-key']
		, ($this->email ? ':add_email' : '')
		);

		$digit = $this->app_play_and_get_digits ( 1, 1, 1, 5000
		, $this->voicemail_params[$this->voicemail_context]['terminator-key'], $phrase
		, 'ivr/ivr-that_was_an_invalid_entry.wav', 'TMP', '\\\d+'
		);

		digit_choice:
		$this->read_message ( $msg->uuid );
		switch ($digit) {
			case $this->voicemail_params[$this->voicemail_context]['listen-file-key']:
				$this->play_message($msg);
				break;

			case $this->voicemail_params[$this->voicemail_context]['save-file-key']:
				break;

			case $this->voicemail_params[$this->voicemail_context]['delete-file-key']:
				$this->delete_message ( $msg->uuid );
				break;

			case $this->voicemail_params[$this->voicemail_context]['email-key']:
				break;

			case $this->voicemail_params[$this->voicemail_context]['forward-key']:
				$this->forward($msg);
				break;

			case $this->voicemail_params[$this->voicemail_context]['callback-key']:
				$this->callback($msg);
				break;
		}
	}
	
	public function email($msg) {
		// get email address and send mail with attachment
	}
	
	public function forward($msg) {
		// ask who to forward to, then voicemail_inject
		$this->api_voicemail_inject($fwd2, $msg->file_path, $msg->cid_num, $msg->cid_name);
	}
	
	public function callback($msg) {
		$this->api_uuid_transfer($this->CHANNEL_DATA->{'Unique-ID'}, $msg->cid_num
		, $this->voicemail_params[$this->voicemail_context]['callback-dialplan']
		, $this->voicemail_params[$this->voicemail_context]['callback-context']
		);
	}

	public function play_message_count($folder) {
		$this->debug("Folder [$folder]");
		$phrase = sprintf('phrase:voicemail_message_count:%s:%s', count($this->msgs[$folder]), $folder);
		$this->app_playback($phrase);
	}

	public function read_message($uuid, $is_read = true) {
		$this->api_vm_read ( $this->mailbox, ($is_read ? "read" : "unread"), $uuid );
	}

	public function delete_message($uuid) {
		$this->api_vm_delete ( $this->mailbox, $uuid );
	}

	public function load_voicemail_params() {
		$this->debug(sprintf("Is Array: %d", is_array($this->voicemail_conf->profiles->profile)));
		foreach ($this->voicemail_conf->profiles->profile as $profile) {
			//$this->debug(print_r($profile, true));
			$this->debug($profile['name']);
			foreach ($profile->param as $param) {
				$this->debug(sprintf('[%s] %s => %s', $profile['name'], $param['name'], $param['value']));
				$profile_name = (string) $profile['name'];
				$param_name = (string) $param['name'];
				$param_value = (string) $param['value'];
				$this->voicemail_params[$profile_name][$param_name] = $param_value;
			}
		}
	}

	public function voicemail_config_menu() {
		$phrase = sprintf('phrase:voicemail_config_menu:%s:%s:%s:%s:%s'
		, $this->voicemail_params[$this->voicemail_context]['record-greeting-key']
		, $this->voicemail_params[$this->voicemail_context]['choose-greeting-key']
		, $this->voicemail_params[$this->voicemail_context]['record-name-key']
		, $this->voicemail_params[$this->voicemail_context]['change-pass-key']
		, $this->voicemail_params[$this->voicemail_context]['main-menu-key']
		);
		for ($i = 0; $i < $this->voicemail_params[$this->voicemail_context]['max-retries']; $i++) {
			$logdata = sprintf("Attempt %d of %d"
			, $i+1, $this->voicemail_params[$this->voicemail_context]['max-retries']
			);
			$this->debug($logdata);
			$this->console_log('INFO', $logdata);
			$choice = $this->app_play_and_get_digits(1, 1, 1, 5000
			, $this->voicemail_params[$this->voicemail_context]['terminator-key'], $phrase
			, 'ivr/ivr-that_was_an_invalid_entry.wav', 'OPT', '\\\d'
			);
			switch ($choice) {
				case $this->voicemail_params[$this->voicemail_context]['record-greeting-key']:
					break;

				case $this->voicemail_params[$this->voicemail_context]['choose-greeting-key']:
					break;

				case $this->voicemail_params[$this->voicemail_context]['record-name-key']:
					break;

				case $this->voicemail_params[$this->voicemail_context]['change-pass-key']:
					break;

				case $this->voicemail_params[$this->voicemail_context]['main-menu-key']:
					break;
			}
		}
	}

	public function voicemail_main_menu() {
		$phrase = sprintf('phrase:voicemail_menu:%s:%s:%s:%s'
		, $this->voicemail_params[$this->voicemail_context]['play-new-messages-key']
		, $this->voicemail_params[$this->voicemail_context]['play-saved-messages-key']
		, $this->voicemail_params[$this->voicemail_context]['config-menu-key']
		, $this->voicemail_params[$this->voicemail_context]['terminator-key']
		);
		for ($i = 0; $i < $this->voicemail_params[$this->voicemail_context]['max-retries']; $i++) {
			$logdata = sprintf("Attempt %d of %d"
			, $i+1, $this->voicemail_params[$this->voicemail_context]['max-retries']
			);
			$this->debug($logdata);
			$this->console_log('INFO', $logdata);
			$choice = $this->app_play_and_get_digits(1, 1, 1, 5000
			, $this->voicemail_params[$this->voicemail_context]['terminator-key'], $phrase
			, 'ivr/ivr-that_was_an_invalid_entry.wav', 'OPT', '\\\d'
			);
			switch ($choice) {
				case $this->voicemail_params[$this->voicemail_context]['play-new-messages-key']:
					$this->play_message_count('new');
					$this->play_messages('new');
					break;

				case $this->voicemail_params[$this->voicemail_context]['play-saved-messages-key']:
					$this->play_message_count('saved');
					$this->play_messages('saved');
					break;

				case $this->voicemail_params[$this->voicemail_context]['config-menu-key']:
					$this->voicemail_config_menu();
					break;

				case $this->voicemail_params[$this->voicemail_context]['terminator-key']:
					return;
			}
		}
	}

	public function main() {
		$this->voicemail_conf = $this->config_voicemail();
		$this->debug(print_r($this->voicemail_conf, true));
		$this->load_voicemail_params();
		$this->debug(print_r($this->voicemail_params, true));

		if (!$this->vm_auth()) {
			return;
		}
		$this->debug ( "VM Authenticated" );
		$this->mailbox = sprintf ( '%s@%s', $this->ext, $this->domain );
		$this->debug ( sprintf ( 'Mailbox: [%s]', $this->mailbox ) );
		$this->get_messages();

		$this->play_message_count ('new');
		$this->play_message_count('saved');
		$this->voicemail_main_menu();

	}

}

$call = new IVR_Voicemail();
$call->main();







