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

class MediaBrotha_Backend extends MediaBrotha_Media implements Iterator {
	protected $_buffer = Array();
	public function __construct($args = Array()) {
		parent::__construct($args);
		$this->register();
		$this->setMimeType('application/x-mediabrotha-backend-'.strtolower($this->getName()));
		MediaBrotha_Core::registerMimeType($this, 'application/x-mediabrotha-backend-'.strtolower($this->getName()));
		$this->setMetadata('name', $this->getName());
		$this->setMetadata('uri', 'MediaBrotha_Backend://'.$this->getName());
	}

	public function register() {
		MediaBrotha_Core::registerBackend($this);
	}

	public function getName() {
		return preg_replace('/^MediaBrotha_Backend_(.*)$/', '$1', get_class($this));
	}

	public function capabilities($uri = NULL, $mime_type = NULL, $mime_encoding = NULL) {
		return Array();
	}

	public function isHidden() {
		return parent::isHidden() || !in_array('browse', $this->capabilities());
	}

	// Capability browse
	public function fetch($uri) {
		return false;
	}

	public function rewind() {
		reset($this->_buffer);
	}

	public function current() {
		return current($this->_buffer);
	}

	public function key() {
		return key($this->_buffer);
	}

	public function next() {
		return next($this->_buffer);
	}

	public function valid() {
		$key = key($this->_buffer);
		$var = ($key !== NULL && $key !== FALSE);
		return $var;
	}

	// Capability play
	public function play($media) {
		return false;
	}
	public function pause() {
		return false;
	}
	public function stop() {
		return false;
	}
	// Capability playlist
	public function playlistEnqueue($media) {
		return false;
	}
	public function playlistNext($media) {
		return false;
	}
	public function playlistPrevious($media) {
		return false;
	}
}

