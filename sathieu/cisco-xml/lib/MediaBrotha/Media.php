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

final class MediaBrotha_Media {
	private $_URI = NULL;
	private $_metadata = Array();
	private $_mimeType = NULL;
	private $_mimeEncoding = NULL;

	private $_parent = NULL;

	public function __construct($URI, array $metadata = Array(), $mime_type = NULL, $mime_encoding = NULL) {
		 $this->_URI = $URI;
		 $this->setMetadata($metadata);
		 $this->setMimeType($mime_type);
		 $this->setMimeEncoding($mime_encoding);
	}

	/* URI */
	public function getURI() {
		return $this->_URI;
	}

	/* metadata */
	public function getMetadata($name = NULL, $max_length = NULL) {
		if ($name === NULL) {
			return $this->_metadata;
		} elseif (isset($this->_metadata[$name])) {
			if ($max_length) {
				return substr($this->_metadata[$name], 0, $max_length);
			} else {
				return $this->_metadata[$name];
			}
		} else {
			return NULL;
		}
	}
	public function setMetadata($name, $value = NULL) {
		if (is_array($name)) {
			$this->_metadata = $name;
		} else {
			switch($name) {
				case 'hidden':
				case 'display_name':
				case 'Array':
				//music
				case 'artist':
				case 'album':
					break;
				default:
					print "Unusual metadata '$name'.\n";
			}
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

	/* parent */
	public function getParent() {
		return $this->_parent;
	}
	public function setParent($parent) {
		$this->_parent = $parent;
	}

	/* Most used metadata */
	public function getDisplayName($max_length = NULL) {
		if ($name = $this->getMetadata('display_name')) {
			if ($max_length) {
				return substr($name, 0, $max_length);
			} else {
				return $name;
			}
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

