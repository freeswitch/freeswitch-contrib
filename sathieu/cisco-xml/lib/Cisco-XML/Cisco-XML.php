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
class CiscoIPPhone {
	static public function userAgentIsCiscoIPPhone() {
		if (isset($_SERVER['HTTP_USER_AGENT'])) {
			return preg_match('/^Allegro-Software-WebClient\//', $_SERVER['HTTP_USER_AGENT']) || isset($_GET['forceciscoxml']);
		}

	}
}

class CiscoXMLField {
	private $_allowed_fields = NULL;
	private $_xml = NULL;
	
	public function __construct($tag_name, $value = NULL, $parent = NULL, $allowed_fields = Array()) {
		$this->_allowed_fields = $allowed_fields;
		if (is_a($parent, 'DOMDocument')) {
			$doc = $parent;
		} else {
			$doc = $parent->ownerDocument;
		}
		$this->_xml = $doc->createElement($tag_name);
		if (is_array($value)) {
			$this->setCiscoElements($value);
		} else {
			$this->_xml->nodeValue = htmlspecialchars($value);
		}
		$parent->appendChild($this->_xml);
	}

	public function setCiscoElements($args) {
		foreach ($args as $k => $v) {
			$this->setCiscoElement($k, $v);
		}
	}

	public function setCiscoElement($name, $value) {
		$short_name = preg_replace('/(:[^:]+)$/','', $name);
		if (!array_key_exists($short_name, $this->_allowed_fields))
			return;
		return new CiscoXMLField($short_name, $value, $this->_xml, $this->_allowed_fields[$short_name]);
	}
}

class CiscoXMLObject {
	protected $_allowed_fields = NULL;
	private $_xml = NULL;
	private $_root = NULL;

	/*
	 * Constructor
	 */
	public function __construct($args = Array()) {
		if (!$this->_allowed_fields) {
			die("Error: Unknow CiscoXMLObject '".get_class($this)."'");
		}
		$this->addAllowedFields();
		$this->_xml = new DOMDocument('1.0', 'utf-8');
		$this->_root = new CiscoXMLField(get_class($this), NULL, $this->_xml, $this->_allowed_fields);
		$this->_root->setCiscoElements($args);
	}

	protected function addAllowedFields() {
		switch(get_class($this)) {
			case 'CiscoIPPhoneStatus':
			case 'CiscoIPPhoneExecute':
				break;
			default:
				$this->_allowed_fields['SoftKeyItem'] = Array(
					'Name' => true,
					'URL' => true,
					'URLDown' => true,
					'Position' => true,
					);
		}
	}
	/*
	 * Private methods
	 */
	/*
	 * Public methods
	 */
	public function __toString() {
		return $this->_xml->saveXML();
	}


	public function setCiscoElement($name, $value) {
		return $this->_root->setCiscoElement($name, $value);
	}

	public function setCiscoElements($args) {
		$this->_root->setCiscoElements($args);
	}

	static public function HttpHeader() {
		Header('Content-Type: text/xml');
		header("Connection: close");
		header("Expires: -1");
	}

	public function execute($url) {
		$opts = array('http' =>
			array(
				'method'  => 'POST',
				'header'  => 'Content-type: application/x-www-form-urlencoded',
				'content' => "XML=".urlencode($this)
				)
			);

		$context  = stream_context_create($opts);
		$stream = fopen($url, 'r', false, $context);
		if ($stream !== false) {
			return stream_get_contents($stream);
		}
	}

}

class CiscoIPPhoneMenu extends CiscoXMLObject {
	protected $_allowed_fields = Array(
		'Title' => true,
		'Prompt' => true,
		'MenuItem' => Array(
			'Name' => true,
			'URL' => true),
		);
}

class CiscoIPPhoneText extends CiscoXMLObject {
	protected $_allowed_fields = Array(
		'Title' => true,
		'Prompt' => true,
		'Text' => true,
		);
}

class CiscoIPPhoneInput extends CiscoXMLObject {
	protected $_allowed_fields = Array(
		'Title' => true,
		'Prompt' => true,
		'URL' => true,
		'InputItem' => Array(
			'DisplayName' => true,
			'QueryStringParam' => true,
			'DefaultValue' => true,
			'InputFlags' => true),
		);
}

