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

require_once('HTTP.php');

class MediaBrotha_Frontend_CiscoXML extends MediaBrotha_Frontend_HTTP {
	private $_xml = NULL;
	public function begin($item) {
		$this->_xml = new CiscoIPPhoneMenu();
		$this->_xml->setCiscoElement('Title', $item->getDisplayName(32));
		$this->_xml->setCiscoElement('Prompt', substr($item->getURI(), -32));
		if ($parent = $item->getParent()) {
			$this->addItem($parent);
		}
	}
	public function addItem($item) {
		$url = $this->rootURL().'?'.
			MediaBrotha_Core::value2hash(
				'mime_type='.urlencode($item->getMimeType()).'&'.
				($item->getMimeEncoding() ? ('mime_encoding='.urlencode($item->getMimeEncoding()).'&') : '').
				'uri='.urlencode($item->getURI()));
		$this->_xml->setCiscoElement('MenuItem',
			Array('Name' => $item->getDisplayName(), 'URL' => $url));
	}
	public function finish($item) {
		$pos = 1;
		$this->_xml->setCiscoElement('SoftKeyItem',
			Array('Name' => 'Select',
			'URL' => 'SoftKey:Select',
			'Position' => $pos++));
		foreach (MediaBrotha_Core::getBackends() as $backend) {
			foreach ($backend->getMediaActions(MediaBrotha_Core::getCurrentMedia()) as $action) {
				$this->_xml->setCiscoElement('SoftKeyItem',
					Array('Name' => $action,
					'URL' => 'QueryStringParam:'.MediaBrotha_Core::value2hash('action='.$action.'&backend='.$backend->getBackendName()),
					'Position' => $pos++));
			}
		}
	}
	public function render() {
		if (true) {
			CiscoXMLObject::HttpHeader();
			print $this->_xml;
			//print "\n<!--".strlen($this->_xml).'-->';
		} else {
			print strlen($this->_xml)."\n";
			print $this->_xml->execute($this->_infos['push_url']);
		}
	}
}

