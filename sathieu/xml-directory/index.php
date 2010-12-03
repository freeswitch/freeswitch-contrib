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

header("Content-type: text/xml");
header("Connection: close");
header("Expires: -1");
header("Set-Cookie: ASPSESSIONIDCARBSTTC=ECJOLNNABLFHPLAFGEJMPNFP; path=/");
?>
<CiscoIPPhoneMenu>
	<Prompt>Selectionner un repertoire</Prompt>
<?php
foreach($links as $k => $v) {
?>
	<MenuItem>
		<Name><?php echo ucfirst(l10n($v));?></Name>
		<URL><?php echo $k;?></URL>
	</MenuItem>
<?php
}
?>
</CiscoIPPhoneMenu>
