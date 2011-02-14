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
	static protected $_supported_mime_types = Array(
		'audio/mpeg',
		'video/x-msvideo',
	);

	// Helper functions
	private function send_command($command, array $args = Array()) {
		$url = $this->getParam('http_intf').
			'/requests/status.xml'.
			'?command='.$command;
		foreach ($args as $k => $v) {
			$url.= '&'.$k.'='.rawurlencode($v);
		}
		$output = file_get_contents($url);
		Header('Content-Type: text/xml');
		print $output.'<!--'.$url."-->\n";
	}

	// Actions
	protected function _isHandled(MediaBrotha_Media $media) {
		return in_array($media->getMimeType(), Array($this::$_supported_mime_types))
			|| (substr($media->getURI(), 0, 7) === 'file://');
	}

	public function getMediaActions(MediaBrotha_Media $media) {
		if ($this->_isHandled($media)) {
			return Array(
				'play_media',
				'pause_media',
				'stop_media',
				'enqueue_media',
				'next_media',
				'previous_media',
			);
		} else {
			return parent::getMediaActions($media);
		}
	}
	public function doMediaAction($action, MediaBrotha_Media $media) {
		switch($action) {
			case 'play_media':
				return $this->send_command('in_play', Array('input' => $media->getURI()));
			case 'pause_media':
				return $this->send_command('pl_pause');
			case 'stop_media':
				return $this->send_command('pl_stop');
			case 'enqueue_media':
				return $this->send_command('in_enqueue', Array('input' => $media->getURI()));
			case 'next_media':
				return $this->send_command('pl_next');
			case 'previous_media':
				return $this->send_command('pl_previous');
			default:
				return parent::doMediaAction($action, $media);
		}
	}
}


