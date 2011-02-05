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

require_once('MediaBrotha/Backend.php');
require_once('MediaBrotha/Frontend.php');
require_once('MediaBrotha/Media.php');

class MediaBrotha_Core {
	private static $_frontend = NULL;
	private static $_backends = Array();
	private static $_mime_types = Array();
	private static $_media = NULL;
	
	public static function init() {
		MediaBrotha_Core::loadBackend('Root');
	}
	public static function go() {
		$current_backend = MediaBrotha_Core::getCurrentBackend();
		switch(MediaBrotha_Core::getAction()) {
			case 'play':
				$current_backend->play(MediaBrotha_Core::getCurrentMedia());
				break;
			case 'pause':
				$current_backend->pause(MediaBrotha_Core::getCurrentMedia());
				break;
			case 'stop':
				$current_backend->stop(MediaBrotha_Core::getCurrentMedia());
				break;
			case 'pl_enqueue':
				$current_backend->playlistEnqueue(MediaBrotha_Core::getCurrentMedia());
				break;
			case 'pl_next':
				$current_backend->playlistNext(MediaBrotha_Core::getCurrentMedia());
				break;
			case 'pl_previous':
				$current_backend->playlistPrevious(MediaBrotha_Core::getCurrentMedia());
				break;
			case 'browse':
			default:
				$current_uri = MediaBrotha_Core::getCurrentURI();
				$current_backend->fetch((string) $current_uri);
				MediaBrotha_Core::$_frontend->begin($current_backend);
				foreach($current_backend as $item) {
					if ($item->isHidden()) {
						continue;
					}
					MediaBrotha_Core::$_frontend->addItem($item);
				}
				MediaBrotha_Core::$_frontend->finish($current_backend);
				MediaBrotha_Core::$_frontend->render();
		}
	}

	/*
	 * Backends
	 */
	public static function loadBackend($provider_name, $args = Array()) {
		$class_name = "MediaBrotha_Backend_${provider_name}";
		if (!class_exists($class_name)) {
			require_once("MediaBrotha/Backend/${provider_name}.php");
		}
		return new $class_name($args);
	}
	public static function registerBackend($backend) {
		MediaBrotha_Core::$_backends[] = $backend;
	}
	public static function registerMimeType($backend, $mime_type) {
		MediaBrotha_Core::$_mime_types[$mime_type][] = $backend;
	}
	public static function getBackends() {
		return MediaBrotha_Core::$_backends;
	}
	public static function getCurrentBackend() {
		if (!empty($_GET['mime_type']) && !empty(MediaBrotha_Core::$_mime_types[$_GET['mime_type']])) {
			return MediaBrotha_Core::$_mime_types[$_GET['mime_type']][0];
		} else {
			return MediaBrotha_Core::$_backends[0];
		}
	}
	/*
	 * Frontend
	 */
	public static function connectFrontend($provider_name, $args = Array()) {
		if (!$provider_name) {
			return;
		}
		$args['provider'] = $provider_name;
		$class_name = "MediaBrotha_Frontend_$provider_name";
		if (!class_exists($class_name)) {
			require_once("MediaBrotha/Frontend/${provider_name}.php");
		}
		MediaBrotha_Core::$_frontend = new $class_name($args);
	}
	/*
	 * ...
	 */
	public static function getAction() {
		if (!empty($_GET['action'])) {
			return $_GET['action'];
		} else {
			return 'browse';
		}
	}

	public static function getCurrentURI() {
		if (!empty($_GET['uri'])) {
			return (string) $_GET['uri'];
		}
	}
	public static function getCurrentMedia() {
		if (empty(MediaBrotha_Core::$_media)) {
			MediaBrotha_Core::$_media = new MediaBrotha_Media();
			MediaBrotha_Core::$_media->setURI(MediaBrotha_Core::getCurrentURI());
		}
		return MediaBrotha_Core::$_media;
	}

}

