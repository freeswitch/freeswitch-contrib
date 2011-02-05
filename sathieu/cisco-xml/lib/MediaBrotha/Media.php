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

	private $_URI = NULL;
	private $_mimeType = NULL;
	private $_mimeEncoding = NULL;

	public function __construct($URI, array $metadata = Array(), $mime_type = NULL, $mime_encoding = NULL) {
		 $this->setURI($URI);
		 $this->setMetadata($metadata);
		 $this->setMimeType($mime_type);
		 $this->setMimeEncoding($mime_encoding);
	}

	/* URI */
	public function getURI() {
		return $this->_URI;
	}
	public function setURI($URI) {
		$this->_URI = $URI;
	}

	/* metadata */
	public function getMetadata($name = NULL) {
		if ($name === NULL) {
			return $this->_metadata;
		} elseif (isset($this->_metadata[$name])) {
			return $this->_metadata[$name];
		} else {
			return NULL;
		}
	}
	public function setMetadata($name, $value = NULL) {
		if (is_array($name)) {
			$this->_metadata = $name;
		} else {
			$this->_metadata[$name] = $value;

		}
	}

	/* mimeType */
	public function getMimeType() {
		return $this->_mimeType;
	}
	public function setMimeType($mime_type) {
		$this->_mimeType = $mime_type;
	}

	/* mimeEncoding */
	public function getMimeEncoding() {
		return $this->_mimeEncoding;
	}
	public function setMimeEncoding($mime_encoding) {
		$this->_mimeEncoding = $mime_encoding;
	}

	/* Most used metadata */
	public function getDisplayName($max_length = 0) {
		if ($name = $this->getMetadata('name')) {
			return $name;
		} else {
			return '?';
		}
	}
	public function getURIComponent($component) {
		return parse_url($this->getURI($component));
	}
	public function isHidden() {
		return (bool) $this->getMetadata('hidden');
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
