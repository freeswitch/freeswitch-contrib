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

class MediaBrotha_Field {
	private $_attrs;

	public function __construct(array $attrs) {
		$this->_attrs = $attrs;
	}

	public function get($name) {
		if (isset($this->_attrs[$name])) {
			return $this->_attrs[$name];
		}	
	}
}

class MediaBrotha_Form extends ArrayIterator {
	private $_title;
	public function getTitle() {
		return $this->_title;
	}

	public function setTitle($title) {
		$this->_title = $title;
	}

	public function addField(array $args) {
		$this[] = new MediaBrotha_Field($args);
	}
}
