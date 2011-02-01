<?php
/*
Copyright 2011 Mathieu Parent <math.parent@gmail.com> 

This file is part of XML services for Cisco IP Phones.

	XML services for Cisco IP Phones is free software: you can
	redistribute it and/or modify it under the terms of the GNU General
	Public License as published by the Free Software Foundation, either
	version 3 of the License, or (at your option) any later version.

	XML services for Cisco IP Phones is distributed in the hope that it
	will be useful, but WITHOUT ANY WARRANTY; without even the implied
	warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See
	the GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with XML services for Cisco IP Phones. If not, see
	<http://www.gnu.org/licenses/>.

*/

/**
 * @author Mathieu Parent
 */

//* Debug
ini_set('error_reporting', E_ALL | E_STRUCT);
ini_set('display_errors', 1);
// */

ini_set('include_path', ini_get('include_path').PATH_SEPARATOR.'../lib');
require('Cisco-XML/Cisco-XML.php');
require_once('MediaBrotha/Core.php');

MediaBrotha_Core::init();
MediaBrotha_Core::connectFrontend('CiscoXML', Array('push_url' => 'http://cisco:cisco@192.168.0.220/CGI/Execute'));
MediaBrotha_Core::loadBackend('FileSystem', Array('base_path' => '/home/share/music/'));
MediaBrotha_Core::loadBackend('VLC', Array('http_intf' => 'http://127.0.0.1:8080'));

MediaBrotha_Core::go();



