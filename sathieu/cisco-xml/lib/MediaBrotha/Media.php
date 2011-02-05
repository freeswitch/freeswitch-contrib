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

class MediaBrotha_Media {
	private $_metadata = Array();
	private $_mimeType = NULL;
	private $_mimeEncoding = NULL;
	public function __construct(array $metadata = Array()) {
		 $this->_metadata = $metadata;
	}
	public function setMimeType($mime_type) {
		$this->_mimeType = $mime_type;
	}
	public function getMimeType() {
		// TODO
		return $this->_mimeType;
	}
	public function setMimeEncoding($mime_encoding) {
		$this->_mimeEncoding = $mime_encoding;
	}
	public function getMimeEncoding() {
		// TODO
		return $this->_mimeEncoding;
	}
	public function getMetadata($name = NULL) {
		if ($name === NULL) {
			return $this->_metadata;
		} elseif (isset($this->_metadata[$name])) {
			return $this->_metadata[$name];
		} else {
			return NULL;
		}
	}
	public function setMetadata($name, $value) {
		if (is_array($name)) {
			$this->_metadata = $name;
		} else {
			$this->_metadata[$name] = $value;

		}
	}
	/* Most used metadata */
	public function getDisplayName($max_length = 0) {
		return $this->getMetadata('name');
	}
	public function getURI() {
		return $this->getMetadata('uri');
	}
	public function setURI($value) {
		return $this->setMetadata('uri', $value);
	}
	public function getURIComponent($component) {
		return parse_url($this->getURI($component));
	}
	public function isHidden() {
		return isset($this->_metadata['hidden']) && $this->_metadata['hidden'];
	}
}

/*

Mandatory metadata:
uri
name

Usual metadata:
artist
album
*/
