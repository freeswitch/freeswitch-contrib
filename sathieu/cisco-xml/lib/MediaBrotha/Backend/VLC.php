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

class MediaBrotha_Backend_VLC extends MediaBrotha_Backend {
	public function register() {
		parent::register();
		MediaBrotha_Core::registerMimeType($this, 'audio/mpeg');
		MediaBrotha_Core::registerMimeType($this, 'video/x-msvideo');
	}

	public function fetch($uri) {
		$this->_buffer[] = new MediaBrotha_Media();
		return true;
	}
	public function capabilities($uri = NULL, $mime_type = NULL, $mime_encoding = NULL) {
		return Array(
			'play',
			'playlist',
		);
	}
	private function send_command($command, $args = Array()) {
		$url = $this->getMetadata('http_intf').
			'/requests/status.xml'.
			'?command='.$command;
		foreach ($args as $k => $v) {
			$url.= '&'.$k.'='.rawurlencode($v);
		}
		return fopen($url, 'r');
	}

	// Capability play
	public function play($media) {
		return $this->send_command('in_play', Array('input' => $media->getURI()));
	}
	public function pause() {
		return $this->send_command('pl_pause');
	}
	public function stop() {
		return $this->send_command('pl_stop');
	}
	// Capability playlist
	public function playlistEnqueue($media) {
		return $this->send_command('in_enqueue', Array('input' => $media->getURI()));
	}
	public function playlistNext($media) {
		return $this->send_command('pl_next');
	}
	public function playlistPrevious($media) {
		return $this->send_command('pl_previous');
	}
}


