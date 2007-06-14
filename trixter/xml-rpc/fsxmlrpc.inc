<?php
/* 
 * FreeSWITCH Modular Media Switching Software Library / Soft-Switch Application
 * Copyright (C) 2005/2006, Anthony Minessale II <anthmct@yahoo.com>
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
 * The Original Code is FreeSWITCH(tm) PHP5 XMLRPC convienience library
 *
 * The Initial Developer of the Original Code is
 * Bret McDanel <trixter AT 0xdecafbad.com>
 * Portions created by the Initial Developer are Copyright (C)
 * the Initial Developer. All Rights Reserved.
 *
 * Thanks to Ken Rice for showing me a simple xmlrpc example which enabled me to write this
 *
 * Contributor(s):
 * 
 *
 * fsxmlrpc.inc
 *
 */

include_once("xmlrpc.inc");

class fsxmlrpc extends xmlrpc_client {
  function sendRequest($msg='') {
	if($msg == '') return false;

        $resp=$this->send($msg);

	if(!$resp) {
		echo "Communication error: " . $this->client->errstr;
	} else if (!$resp->faultCode()) {
		return(explode("\n" , $resp->value()->scalarval()));
	} else {
                print "An error occurred: ";
                print "Code: " . htmlspecialchars($resp->faultCode()) .
                      " Reason: '" . htmlspecialchars($resp->faultString());
        }

	return false;
  }


  function getChannels() {
	$resp = $this->sendRequest(new xmlrpcmsg('freeswitch.api',
                            array(new xmlrpcval("show", "string"),
                            new xmlrpcval("channels", "string"))));

	if(!$resp) return false;

	$x=0;
	foreach ($resp as $call) {
	        $session = explode(",", $call);
		if($session[0]!='uuid' && sizeof($session) >= 2) {
			$calls[$x]['uuid']             = $session[0];
			$calls[$x]['created']          = $session[1];
			$calls[$x]['name']             = $session[2];
			$calls[$x]['state']            = $session[3];
			$calls[$x]['cid_name']         = $session[4];
			$calls[$x]['cid_num']          = $session[5];
			$calls[$x]['ip_addr']          = $session[6];
			$calls[$x]['dest']             = $session[7];
			$calls[$x]['application']      = $session[8];
			$calls[$x]['application_data'] = $session[9];
			$calls[$x]['read_codec']       = $session[10];
			$calls[$x]['read_rate']        = $session[11];
			$calls[$x]['write_codec']      = $session[12];
			$calls[$x]['write_rate']       = $session[13];
	                $x++;
	        }
	}
	return($calls);

  }

  function getHelp() {
	$resp = $this->sendRequest(new xmlrpcmsg('freeswitch.api',
                            array(new xmlrpcval("help", "string"),
                            new xmlrpcval("", "string"))));

	if(!$resp) return false;

	$x=0;
	foreach ($resp as $call) {
	        $session = explode(",", $call);
		if($session[0]!='uuid' && sizeof($session) >= 2) {
			$calls[$x]['command']     = $session[0];
			$calls[$x]['args']        = $session[1];
			$calls[$x]['description'] = $session[2];
	                $x++;
	        }
	}
	return($calls);

  }

  function conferenceList() {
	$resp = $this->sendRequest(new xmlrpcmsg('freeswitch.api',
                            array(new xmlrpcval("conference", "string"),
                            new xmlrpcval("list", "string"))));

	if(!$resp) return false;

	for ($i=0,$j=0;$i< count($resp); $i++,$j++) {
		preg_match("/Conference (.*) \((\d+) member.*\)/",$resp[$i],$matches);
		if(isset($matches[1]) && $matches[1]!="") {
			$conference[$j]['title']   = $resp[$i];
			$conference[$j]['name']    = $matches[1];
			$conference[$j]['members'] = $matches[2];

			for ($x=0; $x<$conference[$j]['members']; $x++) {
				$confmember = explode(";",$resp[$i+$x+1]);
				$member[$x]['member_id']  = $confmember[0];
				$member[$x]['channel_id'] = $confmember[1];
				$member[$x]['uuid']       = $confmember[2];
				$member[$x]['cid_name']   = $confmember[3];
				$member[$x]['cid_num']    = $confmember[4];
				$member[$x]['flags']      = $confmember[5];
				$member[$x]['volume_in']  = $confmember[6];
				$member[$x]['volume_out'] = $confmember[7];
				$member[$x]['energy']     = $confmember[8];
			}
			$conference[$j]['sessions'] = $member;
			$i+=$x;
		}
	}

	if($i>0) return($conference);
	return(false);

  }

  function conferenceKick($confname='',$memid='') {
	if($confname=='' || $memid=='') return false;

	$resp = $this->sendRequest(new xmlrpcmsg('freeswitch.api',
                            array(new xmlrpcval("conference", "string"),
                            new xmlrpcval("$confname kick $memid", "string"))));

	if(!$resp) return false;
	for ($i=0;$i< count($resp); $i++) {
	  if(strlen($resp[$i])>2 && $resp[$i]=="OK kicked $memid") return true;
	}
	return false;
  }
	
  function conferenceMute($confname='',$memid='') {
	if($confname=='' || $memid=='') return false;

	$resp = $this->sendRequest(new xmlrpcmsg('freeswitch.api',
                            array(new xmlrpcval("conference", "string"),
                            new xmlrpcval("$confname mute $memid", "string"))));

	if(!$resp) return false;
	for ($i=0;$i< count($resp); $i++) {
	  if(strlen($resp[$i])>2 && $resp[$i]=="OK mute $memid") return true;
	}
	return false;
  }
	
  function conferenceUnmute($confname='',$memid='') {
	if($confname=='' || $memid=='') return false;

	$resp = $this->sendRequest(new xmlrpcmsg('freeswitch.api',
                            array(new xmlrpcval("conference", "string"),
                            new xmlrpcval("$confname unmute $memid", "string"))));

	if(!$resp) return false;
	for ($i=0;$i< count($resp); $i++) {
	  if(strlen($resp[$i])>2 && $resp[$i]=="OK unmute $memid") return true;
	}
	return false;
  }

  function conferenceDeaf($confname='',$memid='') {
	if($confname=='' || $memid=='') return false;

	$resp = $this->sendRequest(new xmlrpcmsg('freeswitch.api',
                            array(new xmlrpcval("conference", "string"),
                            new xmlrpcval("$confname deaf $memid", "string"))));

	if(!$resp) return false;
	for ($i=0;$i< count($resp); $i++) {
	  if(strlen($resp[$i])>2 && $resp[$i]=="OK deaf $memid") return true;
	}
	return false;
  }
	
  function conferenceUndeaf($confname='',$memid='') {
	if($confname=='' || $memid=='') return false;

	$resp = $this->sendRequest(new xmlrpcmsg('freeswitch.api',
                            array(new xmlrpcval("conference", "string"),
                            new xmlrpcval("$confname undeaf $memid", "string"))));

	if(!$resp) return false;
	for ($i=0;$i< count($resp); $i++) {
	  if(strlen($resp[$i])>2 && $resp[$i]=="OK undeaf $memid") return true;
	}
	return false;
  }
	

}
?>