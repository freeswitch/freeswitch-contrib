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

require_once('MediaBrotha/Media.php');

class MediaBrotha_MediaIterator implements Iterator {
	private $_backend = NULL;
	private $_media = NULL;
	private $_handle = NULL;
	protected $_buffer = NULL;

	public function __construct(MediaBrotha_Backend $backend, MediaBrotha_Media $media, $buffer = NULL) {
		$this->_backend = $backend;
		$this->_media = $media;
		if ($buffer) {
			$this->_buffer = $buffer;
		} else {
			$this->_buffer = new ArrayIterator();
		}
	}

	/* buffer */
	public function bufferAdd($media) {
		$this->_buffer[] = $media;
	}

	/* iterator */
	public function rewind() {
		$this->_buffer->rewind();
	}

	public function current() {
		return $this->_backend->mediaFromBufferItem($this->_buffer->current());
	}

	public function key() {
		return $this->_buffer->key();
	}

	public function next() {
		return $this->_backend->mediaFromBufferItem($this->_buffer->next());
	}

	public function valid() {
		return $this->_buffer->valid();
	}
}

