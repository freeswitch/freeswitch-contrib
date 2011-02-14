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

abstract class MediaBrotha_Backend {
	private $_params = NULL;

	public function __construct($params = NULL) {
		$this->setParam($params);
		$this->register();
	}

	/* params */
	final public function getParam($name = NULL) {
		if ($name === NULL) {
			return $this->_params;
		} elseif (isset($this->_params[$name])) {
			return $this->_params[$name];
		} else {
			return NULL;
		}
	}
	final public function setParam($name, $value = NULL) {
		if (is_array($name)) {
			$this->_params = $name;
		} else {
			$this->_params[$name] = $value;

		}
	}

	public function register() {
		MediaBrotha_Core::registerBackend($this);
	}

	public function getBackendName() {
		return preg_replace('/^MediaBrotha_Backend_(.*)$/', '$1', get_class($this));
	}

	// Browsing
	public function mediaFromBufferItem($item) {
		return $item;
	}

	// Actions
	final public function getVisibleMediaActions(MediaBrotha_Media $media) {
		$actions = Array();
		foreach ($this->getMediaActions($media) as $action) {
			if ($action{0} !== '_') {
				$actions[] = $action;
			}
		}
		return $actions;
	}

	public function getMediaActions(MediaBrotha_Media $media) {
		return Array();
	}

	public function doMediaAction($action, MediaBrotha_Media $media) {
		return false;
	}

	// Populate
	public function populateMedia(MediaBrotha_Media $media) {
	}

}

