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
 
class FSConfig {
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

	/*** IVR Methods Methods ***/
	public function getConfigMenus(){
		$query = sprintf("select * from menus");
		$stmt = $this->dbh->query($query);
		$results = $stmt->fetchAll();
		return $results;
	}

	public function getConfigMenuOptions($menu_uid) {
		$query = sprintf("select * from menu_entries where menu_uid = %s", $menu_uid);
		$stmt = $this->dbh->query($query);
		$results = $stmt->fetchAll();
		return $results;
	}
}
