<?php
/*
Copyright 2011 Mathieu Parent <math.parent@gmail.com> 

This file is part of MediaBrotha.

	MediaBrotha is free software: you can
	redistribute it and/or modify it under the terms of the GNU General
	Public License as published by the Free Software Foundation, either
	version 3 of the License, or (at your option) any later version.

	MediaBrotha is distributed in the hope that it
	will be useful, but WITHOUT ANY WARRANTY; without even the implied
	warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See
	the GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with MediaBrotha. If not, see
	<http://www.gnu.org/licenses/>.

*/

/**
 * @author Mathieu Parent
 */

class MediaBrotha_Frontend_CiscoXML extends MediaBrotha_Frontend {
	private $_xml = NULL;
	public function begin($item) {
		$this->_xml = new CiscoIPPhoneMenu();
		$this->_xml->setCiscoElement('Title', $item->getDisplayName());
		//$this->_xml->setCiscoElement('Prompt', 'kk');
	}
	public function addItem($item) {
		$url = 'http://'.$_SERVER['SERVER_NAME'].$_SERVER['SCRIPT_NAME'].'?'.
			'mime_type='.urlencode($item->getMimeType()).'&'.
			($item->getMimeEncoding() ? ('mime_encoding='.urlencode($item->getMimeEncoding()).'&') : '').
			'uri='.urlencode($item->getURI());
		//$url = 'http://'.$_SERVER['SERVER_NAME'].$_SERVER['SCRIPT_NAME'];
		$this->_xml->setCiscoElement('MenuItem',
			Array('Name' => $item->getDisplayName(), 'URL' => $url));
	}
	public function finish() {
		$this->_xml->setCiscoElement('SoftKeyItem',
			Array('Name' => 'Select', 'URL' => 'SoftKey:Select', 'Position' => '1'));
		$this->_xml->setCiscoElement('SoftKeyItem',
			Array('Name' => 'Play', 'URL' => 'QueryStringParam:action=play', 'Position' => '2'));
		$this->_xml->setCiscoElement('SoftKeyItem',
			Array('Name' => 'Pause', 'URL' => 'QueryStringParam:action=pause', 'Position' => '3'));
		$this->_xml->setCiscoElement('SoftKeyItem',
			Array('Name' => 'Stop', 'URL' => 'QueryStringParam:action=stop', 'Position' => '4'));
		$this->_xml->setCiscoElement('SoftKeyItem',
			Array('Name' => 'Enqueue', 'URL' => 'QueryStringParam:action=pl_enqueue', 'Position' => '5'));
		$this->_xml->setCiscoElement('SoftKeyItem',
			Array('Name' => 'Next', 'URL' => 'QueryStringParam:action=pl_next', 'Position' => '6'));
		$this->_xml->setCiscoElement('SoftKeyItem',
			Array('Name' => 'Previous', 'URL' => 'QueryStringParam:action=pl_previous', 'Position' => '7'));
		$this->_xml->setCiscoElement('SoftKeyItem',
			Array('Name' => 'Quit', 'URL' => 'SoftKey:Exit', 'Position' => '8'));
	}
	public function render() {
		CiscoXMLObject::HttpHeader();
		print $this->_xml;
		//$this->_xml->execute($this->_infos['push_url']);
	}
}

