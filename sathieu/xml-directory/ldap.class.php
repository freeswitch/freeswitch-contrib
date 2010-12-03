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

class LDAPConnection {
	private $ds;
	function __contruct() {
	}
	function connect() {
		global $ldap_uri, $ldap_dn, $ldap_password;
		$this->ds = ldap_connect($ldap_uri);
		ldap_set_option($this->ds, LDAP_OPT_PROTOCOL_VERSION, 3); 
		ldap_set_option($this->ds, LDAP_OPT_REFERRALS,0);
		ldap_bind($this->ds, $ldap_dn, $ldap_password);
	}
	function search($base_dn,$filter,$attributes, $attrsonly = 0, $sizelimit = NULL, $timelimit = NULL, $deref = NULL) {
		if($deref === NULL) {$deref = LDAP_DEREF_NEVER;}
		return @ldap_search($this->ds,$base_dn,$filter,$attributes, $attrsonly, $sizelimit, $timelimit);
	}
	function first_entry($sr) {
		return ldap_first_entry($this->ds, $sr);
	}
	function next_entry($entry) {
		return ldap_next_entry($this->ds, $entry);
	}
	function get_dn($entry) {
		return ldap_get_dn($this->ds, $entry);
	}
	function get_values($entry, $attribute) {
		return @ldap_get_values($this->ds, $entry, $attribute);
	}
	function get_values_len($entry, $attribute) {
		return @ldap_get_values_len($this->ds, $entry, $attribute);
	}
	function get_attributes($entry) {
		return ldap_get_attributes($this->ds, $entry);
	}
	
	function errno() {
		return ldap_errno($this->ds);
	}
}
?>
