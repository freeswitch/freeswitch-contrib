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
require_once('MediaBrotha/MediaIterator.php');

class MediaBrotha_Core {
	private static $_frontend = NULL;
	private static $_backends = Array();
	private static $_media = NULL;
	
	/*
	 * Core
	 */
	public static function init() {
		if (!session_start()) {
			die('Unable to start session');
		}
		MediaBrotha_Core::_unhashRequest();
		MediaBrotha_Core::loadBackend('Root');
	}

	public static function go() {
		$current_backend = MediaBrotha_Core::getCurrentBackend();
		$current_media = MediaBrotha_Core::getCurrentMedia();
		if (MediaBrotha_Core::getAction() == 'browse') {
			if ($media_iterator = $current_backend->fetch($current_media)) {
				MediaBrotha_Core::$_frontend->begin($current_media);
				foreach ($media_iterator as $item) {
					if (($item === NULL) || $item->isHidden()) {
						continue;
					}
					MediaBrotha_Core::$_frontend->addItem($item);
				}
				MediaBrotha_Core::$_frontend->finish($current_media);
				MediaBrotha_Core::$_frontend->render($current_media);
			} else {
			}
		} else {
			$current_backend->doMediaAction(MediaBrotha_Core::getAction(), $current_media);
		}
	}

	public function addRootMedia($URI, array $metadata = Array(), $mime_type = NULL, $mime_encoding = NULL) {
		MediaBrotha_Core::$_backends['Root']->addRootMedia($URI, $metadata, $mime_type, $mime_encoding);
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
		MediaBrotha_Core::$_backends[$backend->getBackendName()] = $backend;
	}
	public static function getBackend($name) {
		return MediaBrotha_Core::$_backends[$name];
	}
	public static function getBackends() {
		return MediaBrotha_Core::$_backends;
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
	 * Current *
	 */
	public static function getCurrentBackend() {
		if (!empty($_GET['backend']) && !empty(MediaBrotha_Core::$_backends[$_GET['backend']])) {
			return MediaBrotha_Core::$_backends[$_GET['backend']];
		}
		$current_media = MediaBrotha_Core::getCurrentMedia();
		foreach (MediaBrotha_Core::getBackends() as $backend) {
			foreach ($backend->getMediaActions($current_media) as $action) {
				if ($backend->getBackendName() == 'Root') {
					continue;
				}
				if ($action === MediaBrotha_Core::getAction()) {
					return $backend;
				}
			}
		}
		return MediaBrotha_Core::$_backends['Root'];
	}

	public static function getAction() {
		if (!empty($_GET['action'])) {
			return $_GET['action'];
		} else {
			return 'browse';
		}
	}

	public static function getCurrentMedia() {
		if (empty(MediaBrotha_Core::$_media)) {
			if (!empty($_GET['uri'])) {
				MediaBrotha_Core::$_media = new MediaBrotha_Media($_GET['uri']);
			} else {
				MediaBrotha_Core::$_media = new MediaBrotha_Media('MediaBrotha:///');
			}
			foreach (MediaBrotha_Core::getBackends() as $backend) {
				$backend->populateMedia(MediaBrotha_Core::$_media);
			}
		}
		return MediaBrotha_Core::$_media;
	}

	/*
	 * helper functions
	 */
	/* hashes */
	public static function hash2value($h) {
		if (isset($_SESSION['HASHES'][$h])) {
			return $_SESSION['HASHES'][$h];
		} else {
			return NULL;
		}
	}

	public static function value2hash($v) {
		for($hash_length = 1 ; $hash_length <=41 ; $hash_length++) {
			$h = substr(sha1($v), 0, $hash_length);
			if (!isset($_SESSION['HASHES'][$h]) || ($_SESSION['HASHES'][$h] === $v)) {
				break;
			}
		}
		$_SESSION['HASHES'][$h] = $v;
		return $h;
	}

	public static function hash2array($h) {
		if ($v = MediaBrotha_Core::hash2value($h)) {
			parse_str($v, $output);
			return $output;
		}
	}

	private static function _unhashRequest() {
		if (!isset($_SESSION['HASHES'])) {
			$_SESSION['HASHES'] = Array();
		}
		if (isset($_SERVER["QUERY_STRING"])) {
			$tmp = explode('&', $_SERVER["QUERY_STRING"]);
			foreach ($tmp as $h) {
				if (preg_match('/^[a-z0-9]+$/', $h)) {
					$_GET = array_merge($_GET, MediaBrotha_Core::hash2array($h));
				}
			}
		}
	}

}

