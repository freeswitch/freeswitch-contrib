<?php

/* 
 * FreeSWITCH AMF-PHP ESL Client Library
 * Copyright (C) 2009, Ken Rice <krice@tollfreegateway.com>
 *
 * Version: MPL 1.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is FreeSWITCH Modular Media Switching Software Library / Soft-Switch Application
 *
 * The Initial Developer of the Original Code is
 * Ken Rice <krice@tollfreegateway.com>
 * Portions created by the Initial Developer are Copyright (C)
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 * 
 * Ken Rice <krice@tollfreegateway.com>
 *
 * freeswitch.php -- Initial Class file for AMF-PHP FreeSwitch Class using ESL
 *
 ***************************************************************************************************
 * 
 * This Requires the FreeSWITCH ESL Extension to be installed properly on your system.
 *
 */

require_once "ESL.php";
 
class FreeSWITCH {
	var $esl;
	var $dbh;

	private function getDbh(){
		$dbtype='mysql'; 		/* Set the Database type */
		// $db_hostname = 'localhost'; 	/* Database Server hostname */
		$db_hostname = '192.168.1.140'; 	/* Database Server hostname */
		$db_port = '3306';		/* Database Server Port */
		$db_username = 'root'; 		/* Database Server username */
		$db_password = 'password'; 	/* Database Server password */
		$db_database = 'shipment'; 	/* DataBase Name */
		if ($dbtype == 'mysql') {
			$pdo_flags =  array(PDO::MYSQL_ATTR_USE_BUFFERED_QUERY => true,);
		}
		$dbh = new PDO("$dbtype:host=$db_hostname;port=$db_port;dbname=$db_database", $db_username, $db_password, $pdo_flags);
		return $dbh;
	}

        public function __construct() { 
		$esl_server = "127.0.0.1"; 	/* ESL Server */
		$esl_port = "8021"; 		/* ESL Port */
		$esl_secret = "ClueCon"; 	/* ESL Secret */
		$this->esl = new eslConnection($esl_server, $esl_port, $esl_secret);
	}

	/*** General Whats happening Methods ***/

	public function getStatus() {
		$e = $this->esl->sendRecv("api status");
		$body = $e->getBody();
		return $body;
	}

	public function getChannels() {
		$e = $this->esl->sendRecv("api show channels");
		$body = $e->getBody();
		$temp = explode  ("\n", $body);
		$total_count = sizeof($temp);
		$i = -1;
		foreach($temp as $row){
			if ($i == -1) { 
				$header = explode(",", $row);
			} else {
				$temp_row = explode(",", $row);
				$x=0;
				foreach($temp_row as $col){
					$data[$i][$header[$x]] = $col;
					$x++;
				}
			}
			$i++;
			if ($i == $total_count - 3){
				break;
			}
		}
		if ($none != 1) {
			return $data;
		} else { 
			return "OK: No Calls";
		}
	}
	
	public function getCalls() {
		$e = $this->esl->sendRecv("api show calls");
		$body = $e->getBody();
		$temp = explode  ("\n", $body);
		$total_count = sizeof($temp);
		$i = -1;
		foreach($temp as $row){
			if ($i == -1) { 
				$header = explode(",", $row);
			} else {
				$temp_row = explode(",", $row);
				$x=0;
				foreach($temp_row as $col){
					$data[$i][$header[$x]] = $col;
					$x++;
				}
			}
			$i++;
			if ($i == $total_count - 3){
				break;
			}
		}
		return $data;
	}

	/*** General API Command Methods ***/

	public function originate($call_url, $exten, $dialplan = "XML", $context= "default", $cid_name = "amf_php", $cid_number = "888", $timeout="30"){
		$dialstring = "api originate $call_url $exten $dialplan $context $cid_name $cid_number $timeout";
		$e = $this->esl->sendRecv($dialstring);
		$body = $e->getBody();
		return $body;
	}

	public function killUuid($uuid) {
		$e = $this->esl->sendRecv("api uuid_kill $uuid");
		$body = $e->getBody();
		return $body;
	}
	
	/*** Conference Methods ***/
	
	public function kickConferenceUser($conference, $member) {
		$e = $this->esl->sendRecv("api conference $conference kick $member");
		$body = $e->getBody();
		return $body;
	}

	public function getConferenceList() {
		$e = $this->esl->sendRecv("api conference list");
		$body = $e->getBody();
		$data=explode("\n", $body);
		$y=0;
		foreach($data as $row){
        		if(substr($row, 0, 10)=="Conference"){
                		$temp_data = explode(" ", $row);
                		$conf_data[$y] = $temp_data[1];
                		$y++;
        		}
		}
		return $conf_data;
	}

	public function getConferenceUsers($conference_name) {
		$e = $this->esl->sendRecv("api conference $conference_name list");
		$body = $e->getBody();
		$data=explode("\n", $body);
		$y=0;
		foreach($data as $row){
        		if ($row!="" && substr($row, 0, 10) != "Conference"){
                		$temp_data = explode(";", $row);
				$conf_data[$y]['id'] = $temp_data[0];
				$conf_data[$y]['channel'] = $temp_data[1];
				$conf_data[$y]['uuid'] = $temp_data[2];
				$conf_data[$y]['caller_name'] = $temp_data[3];
				$conf_data[$y]['caller_number'] = $temp_data[4];
				$conf_data[$y]['flags'] = $temp_data[5];
				$conf_data[$y]['gain'] = $temp_data[6];
				$conf_data[$y]['volume'] = $temp_data[7];
				$conf_data[$y]['noise'] = $temp_data[8];
				$conf_data[$y]['hear'] = 0;
				$conf_data[$y]['speak'] = 0;
				$conf_data[$y]['talk'] = 0;
				$conf_data[$y]['video'] = 0;
				$conf_data[$y]['floor'] = 0;
				$temp_flags = explode("|", $temp_data[5]);
				foreach ($temp_flags as $flag){
					if ($flag == "hear") $conf_data[$y]['hear'] = 1;
					if ($flag == "speak") $conf_data[$y]['speak'] = 1;
					if ($flag == "talk") $conf_data[$y]['talk'] = 1;
					if ($flag == "video") $conf_data[$y]['video'] = 1;
					if ($flag == "floor") $conf_data[$y]['floor'] = 1;
				}
                		$y++;
        		}
		}
		return $conf_data;
	}

	public function getConfPlayfiles($list_dir) {
		if($list_dir != "") {
			$templist = glob($list_dir);
		} else {
			$templist = glob("/usr/local/freeswitch/sounds/en/us/callie/*/*/*");
		}
		$x=0;
		foreach($templist as $file){
			$temp_file = explode("/", $file);
			$filelist[$x]['label'] = $temp_file[count($temp_file)-1];
			$filelist[$x]['data'] = $file;
			// $filelist[$x] = $file;
			$x++;
		}
		return $filelist;
	}

	public function confPlayfile($conference, $file_path){
		$playfile = "api conference $conference play " . $file_path;
		$e = $this->esl->sendRecv($playfile);
		$body = $e->getBody();
		return $body;
	}

	public function confLock($conference){
		$playfile = "api conference $conference lock";
		$e = $this->esl->sendRecv($playfile);
		$body = $e->getBody();
		return $body;
	}

	public function confUnlock($conference){
		$playfile = "api conference $conference unlock";
		$e = $this->esl->sendRecv($playfile);
		$body = $e->getBody();
		return $body;
	}
	
	public function confMute($conference, $member){
		$playfile = "api conference $conference mute $member";
		$e = $this->esl->sendRecv($playfile);
		$body = $e->getBody();
		return $body;
	}
	
	public function confUnmute($conference, $member){
		$playfile = "api conference $conference unmute $member";
		$e = $this->esl->sendRecv($playfile);
		$body = $e->getBody();
		return $body;
	}

	/*** Directory Methods ***/
	
	/* Directory Domain Methods */
	public function getDirDomains(){
		$dbh = $this->getDbh();
		$query = sprintf("select * from domains");
		$stmt = $dbh->query($query);
		$results = $stmt->fetchAll();
		return $results;
	}

	public function getDirDomain($domains_uid){
		$dbh = $this->getDbh();
		$query = sprintf("select * from domain_params where domains_uid = $domains_uid");
		$stmt = $dbh->query($query);
		$results['params'] = $stmt->fetchAll();
		$query = sprintf("select * from domain_variables where domains_uid = $domains_uid");
		$stmt = $dbh->query($query);
		$results['variables'] = $stmt->fetchAll();
		return $results;
	}
	
	public function addDirDomain($domain_name){
		$dbh = $this->getDbh();
		$query = sprintf('insert into domains (name) values ("%s")', $domain_name);
		return $dbh->exec($query);
	}

	public function addDirDomainParam($domains_uid, $name, $value) {
		$dbh = $this->getDbh();
		$query = sprintf('insert into domain_params (domains_uid, name, value) values (%s, "%s", "%s")', $domains_uid, $name, $value);
		return $dbh->exec($query);
	}

	public function addDirDomainVar($domains_uid, $name, $value) {
		$dbh = $this->getDbh();
		$query = sprintf('insert into domain_variables (domains_uid, name, value) values (%s, "%s", "%s")', $domains_uid, $name, $value);
		$dbh->exec($query);
	}

	public function updateDirDomainParam($param_uid, $name, $value) {
		$dbh = $this->getDbh();
		$query = sprintf('update domain_params set name = "%s", value = "%s" where uid=%s', $name, $value, $param_uid);
		return $dbh->exec($query);
	}

	public function updateDirDomainVar($var_uid, $name, $value) {
		$dbh = $this->getDbh();
		$query = sprintf('update domain_variables set name = "%s", value = "%s" where uid=%s', $name, $value, $var_uid);
		return $dbh->exec($query);
	}

	public function deleteDirDomainParam($param_uid) {
		$dbh = $this->getDbh();
		$query = sprintf('delete from domain_params where uid=%s', $param_uid);
		return $dbh->exec($query);
	}

	public function deleteDirDomainVar($var_uid) {
		$dbh = $this->getDbh();
		$query = sprintf('delete from domain_variables where uid=%s', $var_uid);
		return $dbh->exec($query);
	}


	/* Directory User Methods */
	public function getDirUser($user_uid){
		$dbh = $this->getDbh();
		$query = sprintf("select * from user_params where users_uid = $user_uid");
		$stmt = $dbh->query($query);
		$results['params'] = $stmt->fetchAll();
		$query = sprintf("select * from user_variables where users_uid = $user_uid");
		$stmt = $dbh->query($query);
		$results['variables'] = $stmt->fetchAll();
		return $results;
	}

	public function getDirUsers($domains_uid){
		$dbh = $this->getDbh();
		$query = sprintf("select * from users where domains_uid = $domains_uid");
		$stmt = $dbh->query($query);
		$results = $stmt->fetchAll();
		return $results;
	}

	public function addDirDomainUserParam($users_uid, $name, $value) {
		$dbh = $this->getDbh();
		$query = sprintf('insert into user_params (users_uid, name, value) values (%s, "%s", "%s")', $users_uid, $name, $value);
		return $dbh->exec($query);
	}

	public function addDirDomainUserVar($users_uid, $name, $value) {
		$dbh = $this->getDbh();
		$query = sprintf('insert into user_variables (users_uid, name, value) values (%s, "%s", "%s")', $users_uid, $name, $value);
		$dbh->exec($query);
	}

	public function updateDirDomainUserParam($param_uid, $name, $value) {
		$dbh = $this->getDbh();
		$query = sprintf('update user_params set name = "%s", value = "%s" where uid=%s', $name, $value, $param_uid);
		return $dbh->exec($query);
	}

	public function updateDirDomainUserVar($var_uid, $name, $value) {
		$dbh = $this->getDbh();
		$query = sprintf('update user_variables set name = "%s", value = "%s" where uid=%s', $name, $value, $var_uid);
		return $dbh->exec($query);
	}

	public function deleteDirDomainUserParam($param_uid) {
		$dbh = $this->getDbh();
		$query = sprintf('delete from user_params where uid=%s', $param_uid);
		if ($dbh->exec($query) < 1) {
			return "FAILED " . $query;
		} else {
			return "SUCESS";
		}
	}

	public function deleteDirDomainUserVar($var_uid) {
		$dbh = $this->getDbh();
		$query = sprintf('delete from user_variables where uid=%s', $var_uid);
		if ($dbh->exec($query) < 1) {
			return "FAILED " . $query;
		} else {
			return "SUCESS";
		}
	}

	/* Directory Group Methods */
	public function getDirGroups($domains_uid){
		$dbh = $this->getDbh();
		$query = sprintf("select * from groups where domains_uid = $domains_uid");
		$stmt = $dbh->query($query);
		$results = $stmt->fetchAll();
		return $results;
	}

	public function getDirGroup($groups_uid){
		$dbh = $this->getDbh();
		$query = sprintf("select a.uid as groupMemberUid, a.users_uid as usersUid, b.username as usersUsername from group_members as a, users as b where a.groups_uid = $groups_uid and a.users_uid = b.uid") ;
		$stmt = $dbh->query($query);
		$results['members'] = $stmt->fetchAll();
		$query = sprintf("select uid as usersUid, username as usersUsername from users where uid not in (select users_uid from group_members where groups_uid = $groups_uid) and domains_uid = (select domains_uid from groups where uid = $groups_uid)");
		$stmt = $dbh->query($query);
		$results['nonmembers'] = $stmt->fetchAll();
		return $results;
	}

	public function addDirGroup($domains_uid, $new_groupname){
		$dbh = $this->getDbh();
		$query = sprintf("insert into groups (domains_uid, name) values (%s, '%s')", $domains_uid, $new_groupname);
		
		if ($dbh->exec($query) > 0){
			return "INSERTED";
		} else { 
			return "FAILED $query";
		}
	}

}
/* For Emacs:
 * Local Variables:
 * mode:c
 * indent-tabs-mode:t
 * tab-width:4
 * c-basic-offset:4
 * End:
 * For VIM:
 * vim:set softtabstop=4 shiftwidth=4 tabstop=4 expandtab:
 */
?>
