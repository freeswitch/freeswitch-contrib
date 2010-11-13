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

$base_url = 'http://'.$_SERVER['SERVER_NAME'].'/xml-directory';
$links = Array(
	"$base_url/directory.php?action=search"=>'Default directory',
	"$base_url/directory.php?action=search&amp;directory=ldap2"=>'LDAP 2'
	);

$directories = Array(
	'default' => Array( // OpenLDAP example
		'ldap_uri'         => 'ldaps://ldap.example.org/',
		'ldap_dn'          => 'uid=proxyuser,ou=people,dc=example,dc=org',
		'ldap_password'    => 'proxypass',
		'base_dn'          => 'dc=example,dc=org',
		'base_filter'      => 
			'(&(objectClass=person)'. // personne
			'(!(cn=*$))'. // no winstations
			'(!(objectClass=gosaUserTemplate))'. // no GOsa templates
			'(!(uid=nobody))(!(uid=root))(!(uid=admin))(!(uid=joindom)))', // no nobody,root,admin,joindom
		'sizelimit'        => 20,
		'search_fields'    => Array(
		//ldap attribute => Array(display [, filter])
		//max 5
			'sn'              => Array('last name'),
			'givenName'       => Array('first name'),
			'telephoneNumber' => Array('phone', '(telephoneNumber=*%s*)'),
			//'o'               => Array('organization'),
			//'mobile'          => Array('mobile'),
			//'facsimileTelephoneNumber' => Array('fax'),
			//'l'               => Array('location'),
			//'st'              => Array('department'),
			//'roomNumber'      => Array('room number'),
		),
		'detail_fields'    => Array(
		//ldap attribute => display
			'sn'              => 'last name',
			'givenName'       => 'first name',
			'telephoneNumber' => 'phone',
			'mobile'          =>'mobile',
			'facsimileTelephoneNumber'=>'fax',
			'o'               =>'organization',
			'st'              =>'department',
			'l'               =>'location',
			'postalAddress'   =>'postal address',
			'roomNumber'      =>'room number',
			'mail'            => 'mail',
		),

		'phone_maps'       => Array(
			'/015368(\d{4})/' => '\1',
			'/014475(\d{4})/' => '\1',
			'/^(.*)$/'        => '0\1',
		),

		'phone_fields'     => Array(
			'facsimileTelephoneNumber',
			'mobile',
			'pager',
			'telephoneNumber',
		),
		'default_vm_password' => '12345',
	),
	'ldap2' => Array( // Another anonymous one
		'ldap_uri'         => 'ldap://10.11.12.13/',
		//'ldap_dn'          => '',
		//'ldap_password'    => '',
		'base_dn'          => 'o=example, o=directoryRoot',
		'base_filter'      => 
			'(objectClass=person)',
		'sizelimit'        => 20,
		'search_fields'    => Array(
		//ldap attribute => Array(display [, filter])
		//max 5
			'sn'              => Array('last name'),
			'givenName'       => Array('first name'),
			'misc15'          => Array('direction', '(misc15=%s)'),
			'internationalisdnnumber' => Array('phone', '(internationalisdnnumber=*%s)'),
		),
		'detail_fields'    => Array(
		//ldap attribute => display
			'sn'              => 'last name',
			'givenName'       => 'first name',
			'internationalisdnnumber' => 'phone',
			'misc15'          => 'direction',
			'misc16'          => 'service',
			'misc17'          => 'affectation',
			'misc37'          => 'detail',
			'postalAddress'   => 'postal address',
		),

		'phone_maps'       => Array(
			'/^(.*)$/'        => '0\1',
		),

		'phone_fields'     => Array(
			'internationalisdnnumber',
		),
	)
);

$translations = Array(
	'fr' => Array(
		'department'=>'département',
		'directory' => 'annuaire',
		'fax' => 'télécopie',
		'first name' => 'prénom',
		'last name' => 'nom',
		'location'=> 'lieu',
		'mail' => 'couriel',
		'organization' => 'organisation',
		'phone' => 'téléphone',
		'postal address'=>'adresse',
		'room number'=>'numéro de bureau',
		//
		'Back' => 'Retour',
		'Details' => 'Détails',
		'Dial' => 'Composer',
		'Fill the fields' => 'Remplissez les champs',
		'Results' => 'Résultats',
		'%d first' => '%d premiers',
		)
	);

function l10n($text) {
	global $translations;
	$locale = 'fr';
	if (array_key_exists($text,$translations[$locale])) {
		return $translations[$locale][$text];
	} else {
		return $text;
	}
}
?>
