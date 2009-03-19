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
 * freeswitch.php -- Initial Class file for XML_CURL responses for Codename: SHIPMENT
 *
 ***************************************************************************************************
 * 
 * This Requires the FreeSWITCH ESL Extension to be installed properly on your system.
 *
 */

// require_once "ESL.php";
 
class FSDirectory {
	// var $esl;

	var $dbh;

	public function __construct(){
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
		$this->dbh = new PDO("$dbtype:host=$db_hostname;port=$db_port;dbname=$db_database", $db_username, $db_password, $pdo_flags);
		
	}

        private function getESL() { 
		$esl_server = "127.0.0.1"; 	/* ESL Server */
		$esl_port = "8021"; 		/* ESL Port */
		$esl_secret = "ClueCon"; 	/* ESL Secret */
		$this->esl = new eslConnection($esl_server, $esl_port, $esl_secret);
	}

	/*
	public function getStatus() {
		$e = $this->esl->sendRecv("api status");
		$body = $e->getBody();
		return $body;
	}
	*/

	/*** Directory Methods ***/
	
	/* Directory Domain Methods */
	public function getDirDomains(){
		$query = sprintf("select * from domains");
		$stmt = $this->dbh->query($query);
		$results = $stmt->fetchAll();
		return $results;
	}

	public function getDirDomainbyName($domain_name){
		$query = sprintf("select * from domains where name = '%s'", $domain_name);
		$stmt = $this->dbh->query($query);
		$results = $stmt->fetchAll();
		return $results[0];
	}

	public function getDirDomain($domains_uid){
		$query = sprintf("select * from domain_params where domains_uid = $domains_uid");
		$stmt = $this->dbh->query($query);
		$results['params'] = $stmt->fetchAll();
		$query = sprintf("select * from domain_variables where domains_uid = $domains_uid");
		$stmt = $this->dbh->query($query);
		$results['variables'] = $stmt->fetchAll();
		return $results;
	}
	
	/* Directory User Methods */
	public function getDirUser($user_uid){
		$query = sprintf("select * from user_params where users_uid = $user_uid");
		$stmt = $this->dbh->query($query);
		$results['params'] = $stmt->fetchAll();
		$query = sprintf("select * from user_variables where users_uid = $user_uid");
		$stmt = $this->dbh->query($query);
		$results['variables'] = $stmt->fetchAll();
		return $results;
	}

	public function getDirUsers($domains_uid){
		$query = sprintf("select * from users where domains_uid = $domains_uid");
		$stmt = $this->dbh->query($query);
		$results = $stmt->fetchAll();
		return $results;
	}

	public function getDirUsersByDomainUidByUsername($domain_uid, $user_name){
		$query = sprintf("select * from users where domains_uid = '%s' and username = '%s'", $domain_uid, $user_name);
		$stmt = $this->dbh->query($query);
		$results = $stmt->fetchAll();
		return $results[0];
	}

	/* Directory Group Methods */
	public function getDirGroups($domains_uid){
		$query = sprintf("select * from groups where domains_uid = $domains_uid");
		$stmt = $this->dbh->query($query);
		$results = $stmt->fetchAll();
		return $results;
	}

	public function getDirGroup($groups_uid){
		$query = sprintf("select a.uid as groupMemberUid, a.users_uid as usersUid, b.username as usersUsername from group_members as a, users as b where a.groups_uid = $groups_uid and a.users_uid = b.uid") ;
		$stmt = $this->dbh->query($query);
		$results['members'] = $stmt->fetchAll();
		$query = sprintf("select uid as usersUid, username as usersUsername from users where uid not in (select users_uid from group_members where groups_uid = $groups_uid) and domains_uid = (select domains_uid from groups where uid = $groups_uid)");
		$stmt = $this->dbh->query($query);
		$results['nonmembers'] = $stmt->fetchAll();
		return $results;
	}

	public function getDirGroupsByDomianUidByUserUid($domain_uid, $user_uid){
		$query = sprintf("select a.uid as groupUid, a.name as groupName, b.uid as usersUid from groups as a, group_members as b where a.uid = b.groups_uid and b.domains_uid = %s and b.users_uid = %s", $domain_uid, $user_uid) ;
		$stmt = $this->dbh->query($query);
		$results = $stmt->fetchAll();
		return $results;
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
