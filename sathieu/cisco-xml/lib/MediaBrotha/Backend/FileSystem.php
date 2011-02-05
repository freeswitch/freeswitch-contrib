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

class MediaBrotha_Backend_FileSystem extends MediaBrotha_Backend {
	public function register() {
		parent::register();
		MediaBrotha_Core::registerMimeType($this, 'application/x-directory');
	}

	public function isURISafe($uri) {
		$real_path = realpath(parse_url($uri, PHP_URL_PATH));
		$real_base_path = realpath($this->getMetadata('base_path'));
		do {
			if ($real_path === $real_base_path) {
				return true;
			}
		} while(($real_path != dirname($real_path)) && $real_path = dirname($real_path));
		return false;
	}

	public function capabilities($uri = NULL, $mime_type = NULL, $mime_encoding = NULL) {
		return Array(
			'browse',
		);
	}
	// Capability browse
	protected function _mediaFromBufferItem($file) {
		if ($file) {
			$media = new MediaBrotha_Media(Array(
				'uri' => 'file://'.realpath($file->getPathname()),
				'name' => $file->getFilename(),
				'hidden' => !$file->isReadable() || preg_match('/^\.([^.]|$)/', $file->getFilename()),
			));
			if ($file->isDir()) {
				$media->setMimeType('application/x-directory');
			} else {
				$finfo = new finfo(FILEINFO_MIME_TYPE);
				$media->setMimeType($finfo->file($file->getPathname()));
				$media->setMimeEncoding($finfo->file($file->getPathname(), FILEINFO_MIME_ENCODING));
			}
			return $media;
		}
	}

	public function fetch($uri) {
		if ((parse_url($uri, PHP_URL_SCHEME) != 'file') || !$this->isURISafe($uri)) {
			$uri = 'file://'.$this->getMetadata('base_path');
		}
		$this->_buffer = new DirectoryIterator(parse_url($uri, PHP_URL_PATH));
		return true;
	}
}

