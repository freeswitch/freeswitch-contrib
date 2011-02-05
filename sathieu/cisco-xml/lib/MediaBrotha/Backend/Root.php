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

class MediaBrotha_Backend_Root extends MediaBrotha_Backend {
	private $_media = NULL;
	private $_media_iterator = NULL;

	public function __construct($params = NULL) {
		parent::__construct($params);
		$this->_media = new MediaBrotha_Media('MediaBrotha:///',
			Array(
				'display_name' => 'Root',
			)
		);
		$this->_media_iterator = new MediaBrotha_MediaIterator($this, $this->_media);
	}

	public function addRootMedia($URI, array $metadata = Array(), $mime_type = NULL, $mime_encoding = NULL) {
		$this->addRootMediaObj(new MediaBrotha_Media($URI, $metadata, $mime_type,$mime_encoding));
	}
	public function addRootMediaObj($media) {
		$this->_media_iterator->bufferAdd($media);
	}

	// Browsing
	public function fetch(MediaBrotha_Media $media) {
		return $this->_media_iterator;
	}
}


