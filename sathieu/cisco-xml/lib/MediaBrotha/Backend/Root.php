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
	public function capabilities($uri = NULL, $mime_type = NULL, $mime_encoding = NULL) {
		return Array(
			'browse',
		);
	}
	public function isHidden() {
		return true;
	}

	// Capability browse
	public function fetch($uri) {
		$this->_buffer = new ArrayIterator();
		foreach(MediaBrotha_Core::getBackends() as $id => $backend) {
			$this->_buffer[] = $backend;
		}
		return true;
	}

}


