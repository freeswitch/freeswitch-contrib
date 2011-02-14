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
	// Helper functions
	public function isURISafe($uri) {
		$real_path = realpath(parse_url($uri, PHP_URL_PATH));
		$real_base_path = realpath($this->getParam('base_path'));
		do {
			if ($real_path === $real_base_path) {
				return true;
			}
		} while(($real_path != dirname($real_path)) && $real_path = dirname($real_path));
		return false;
	}

	// Browsing
	public function mediaFromBufferItem($file) {
		if ($file) {
			if ($file->getFilename() === '..') {
				return NULL;
			}
			$media = new MediaBrotha_Media(
				'file://'.realpath($file->getPathname()),
				Array(
					'display_name' => $file->getFilename(),
					'hidden' => !$file->isReadable() || preg_match('/^\.([^.]|$)/', $file->getFilename()),
				)
			);
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

	// Actions
	protected function _isHandled(MediaBrotha_Media $media) {
		return ($media->getMimeType() === 'application/x-directory')
			|| (substr($media->getURI(), 0, 7) === 'file://');
	}

	public function getMediaActions(MediaBrotha_Media $media) {
		if ($this->_isHandled($media)) {
			return Array(
				'_default',
				'browse',
			);
		} else {
			return parent::getMediaActions($media);
		}
	}

	public function doMediaAction($action, MediaBrotha_Media $media) {
		switch($action) {
			case '_default':
			case 'browse':
				$uri = $media->getURI();
				if ((parse_url($uri, PHP_URL_SCHEME) != 'file') || !$this->isURISafe($uri)) {
					$uri = 'file://'.$this->getParam('base_path');
				}
				$path = parse_url($uri, PHP_URL_PATH);
				if (is_dir($path)) {
					return new MediaBrotha_MediaIterator($this, new DirectoryIterator($path));
				} else {
					return false;
				}
			default:
				return parent::doMediaAction($action, $media);
		}
	}

	// Populate
	public function populateMedia(MediaBrotha_Media $media) {
		if ($this->_isHandled($media)) {
			$parent_path = 'file://'.dirname(parse_url($media->getURI(), PHP_URL_PATH));
			if ($this->isURISafe($parent_path)) {
				$media->setParent(
					new MediaBrotha_Media($parent_path,
						Array(
							'display_name' => '..',
						)
					)
				);
			}
		}
	}

}

