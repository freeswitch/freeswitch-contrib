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

        public function __construct() { 
		$this->esl = new eslConnection('127.0.0.1', '8021', 'ClueCon');
	}
	
	public function getStatus() {
		$e = $this->esl->sendRecv("api status");
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
                		$conf_data[$y] = explode(";", $row);
                		$y++;
        		}
		}

		return $conf_data;
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
			if ($i == $total_count - 4){
				break;
			}
		}
		return $data;
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

	public function originate($call_url, $exten, $dialplan = "XML", $context= "default", $cid_name = "amf_php", $cid_number = "888", $timeout="30"){
		$dialstring = "api originate $call_url $exten $dialplan $context $cid_name $cid_number $timeout";
		$e = $this->esl->sendRecv($dialstring);
		$body = $e->getBody();

		return $dialstring . "\n" . $body;
	}
	
}
?>
