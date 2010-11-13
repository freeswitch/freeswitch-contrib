<?php
/*
Copyright 2007-2010 Mairie de Paris, Service technique de l'eau et de l'assainissement
Copyright 2010 Mathieu Parent <math.parent@gmail.com> 

This file is part of XML Directory for Cisco IP Phones.

	XML Directory for Cisco IP Phones is free software: you can
	redistribute it and/or modify it under the terms of the GNU General
	Public License as published by the Free Software Foundation, either
	version 3 of the License, or (at your option) any later version.

	XML Directory for Cisco IP Phones is distributed in the hope that it
	will be useful, but WITHOUT ANY WARRANTY; without even the implied
	warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See
	the GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with XML Directory for Cisco IP Phones. If not, see
	<http://www.gnu.org/licenses/>.

*/

/**
 * @author Mathieu Parent
 */

require_once('config.inc.php');
require_once('ldap.class.php');
require_once('phone.class.php');

header("Content-type: text/xml");
header("Connection: close");
header("Expires: -1");

if(empty($_GET['directory']) || !isset($directories[$_GET['directory']])) {
	$_GET['directory'] = 'default';
}
extract($directories[$_GET['directory']]);

switch(empty($_GET['action']) ? 'search' : $_GET['action']) {
## ############################################################################
##
case 'search' :
?>
<CiscoIPPhoneInput>
	<Title><?php echo ucfirst(l10n('directory'));?></Title>
	<Prompt><?php echo l10n('Fill the fields');?></Prompt>
	<URL><?php echo "$base_url/directory.php?directory=".$_GET['directory']."&amp;action=result&amp;";?></URL>
<?php
	foreach($search_fields AS $attribute_name=>$infos) {
	?>
	<InputItem>
		<DisplayName><?php echo ucfirst(l10n($infos[0]));?></DisplayName>
		<QueryStringParam><?php echo $attribute_name;?></QueryStringParam>
		<InputFlags><?php echo in_array($attribute_name,$phone_fields) ? 'T' : 'A';?></InputFlags>
		<DefaultValue><?php echo isset($_GET[$attribute_name]) ? $_GET[$attribute_name] : '' ?></DefaultValue>
	</InputItem> 
<?php
	}
	?>
</CiscoIPPhoneInput> 
<?php
	break;
## ############################################################################
##
case 'result' :
	$ldap = new LDAPConnection();
	$ldap->connect();
	$filter=$base_filter;
	foreach($search_fields as $attribute_name => $infos) {
		if(!empty($_GET[$attribute_name])) {
			if(empty($infos[1])) {
				$filter="(&$filter($attribute_name=".$_GET[$attribute_name]."*))";
			} else {
				$filter="(&$filter".sprintf($infos[1], $_GET[$attribute_name]).")";
			}
		}
	}
	$sr= $ldap->search($base_dn,$filter,array('cn','uid'),0, $sizelimit);
	$prompt = l10n('Results');
	if($ldap->errno() == 4) { // LDAP_SIZELIMIT_EXCEEDED
		$prompt.= " (".sprintf(l10n('%d first'), $sizelimit).")";
	}
?>
<CiscoIPPhoneMenu>
	<Title><?php echo ucfirst(l10n('directory'));?></Title>
	<Prompt><?php echo $prompt;?></Prompt>
<?php 
	for ($entryID=$ldap->first_entry($sr);
			$entryID!=false;
			$entryID=$ldap->next_entry($entryID)) {
		$dn= $ldap->get_dn($entryID);
		$dn= preg_replace("/".strtr(preg_quote(', '.$base_dn), Array(' ' => '\s*'))."$/i","",$dn);
		$cn = $ldap->get_values($entryID,'cn');
		$uid = $ldap->get_values($entryID,'uid');
		$uid= $uid[0];
		#compte system, rt ou auditeur
		if(preg_match('/^_.*$/', $uid) or ($uid == 'root') or ($uid == 'nobody')
				or preg_match('/-rt$/', $uid)
				or preg_match('/^[A-Z1-9]{1,3}_[A-Z]{1,8}$/', $uid)) {
			continue;
		}
?>
	<MenuItem>
		<Name><?php echo cisco_encode($cn[0]);?></Name>
		<URL>QueryStringParam:<?php 
		$url="&amp;dn=".urlencode($dn);
		foreach($search_fields as $k=>$v) {
			if(!empty($_GET[$k])) {
				$url.="&amp;$k=".urlencode($_GET[$k])."";
			}
		}
		echo $url;?></URL>
	</MenuItem>
<?php
	}
	?>
	<SoftKeyItem>
		<Name><?php echo l10n('Dial');?></Name>
		<URL><?php echo "$base_url/directory.php?directory=".$_GET['directory']."&amp;action=call"; ?></URL>
		<Position>1</Position>
	</SoftKeyItem>
	<SoftKeyItem>
		<Name><?php echo l10n('Details');?></Name>
		<URL><?php echo "$base_url/directory.php?directory=".$_GET['directory']."&amp;action=detail"; ?></URL>
		<Position>2</Position>
	</SoftKeyItem>
	<SoftKeyItem>
		<Name><?php echo l10n('Back');?></Name>
		<URL><?php 
		$url="$base_url/directory.php?action=search";
		$url.='&amp;directory='.$_GET['directory'];
		foreach($search_fields as $k=>$v) {
			if(!empty($_GET[$k])) {
				$url.="&amp;$k=".urlencode($_GET[$k])."";
			}
		}
		echo $url;?></URL>
		<Position>3</Position>
	</SoftKeyItem> 
	<SoftKeyItem>
		<Name><?php echo l10n('Exit');?></Name>
		<URL>SoftKey:Exit</URL>
		<Position>4</Position>
	</SoftKeyItem> 
</CiscoIPPhoneMenu> 
<?php
	break;
## ############################################################################
##
case 'detail' :
case 'call':
	$ldap = new LDAPConnection();
	$ldap->connect();
	$filter = "(objectClass=*)";
	$dn = $_GET['dn'].", $base_dn";
	$sr= $ldap->search($dn,$filter,array_keys($detail_fields));
?>
<CiscoIPPhoneMenu>
	<Title><?php echo ucfirst(l10n('directory'));?></Title>
	<Prompt><?php echo ucfirst(l10n('details'));?></Prompt>
<?php
	$entry= $ldap->first_entry($sr);
	$attrs = $ldap->get_attributes($entry);
	for ($i=0; $i < $attrs["count"]; $i++) {
		if($_GET['action']=='detail' || ($_GET['action']=='call' && in_array($attrs[$i],$phone_fields))) {
			$value = $ldap->get_values($entry, $attrs[$i]);
	?>
	<MenuItem>
<?php
		if(in_array($attrs[$i],$phone_fields)) {
			echo '		<Name>'.ucfirst(l10n($detail_fields[$attrs[$i]]))." : ".
				phone($value[0]).'</Name>'."\n";
			echo '		<URL>Dial:'.phone($value[0]).'</URL>'."\n";
		} else {
			echo '		<Name>'.ucfirst(l10n($detail_fields[$attrs[$i]]))." : ".
				cisco_encode($value[0]).'</Name>'."\n";
			echo '		<URL></URL>'."\n";
		}
		?>
	</MenuItem>
<?php
		}
	}
	?>
	<SoftKeyItem>
		<Name><?php echo l10n('Dial');?></Name>
		<URL>SoftKey:Select</URL>
		<Position>1</Position>
	</SoftKeyItem>
	<SoftKeyItem>
		<Name><?php echo l10n('Back');?></Name>
		<URL><?php 
		$url="$base_url/directory.php?action=result";
		$url.='&amp;directory='.$_GET['directory'];
		foreach($search_fields as $k=>$v) {
			if(!empty($_GET[$k])) {
				$url.="&amp;$k=".urlencode($_GET[$k])."";
			}
		}
		echo $url;?></URL>
		<Position>3</Position>
	</SoftKeyItem> 
	<SoftKeyItem>
		<Name><?php echo l10n('Exit');?></Name>
		<URL>SoftKey:Exit</URL>
		<Position>4</Position>
	</SoftKeyItem> 
</CiscoIPPhoneMenu>
<?php
	break;
## ############################################################################
##
default:
}
?>
