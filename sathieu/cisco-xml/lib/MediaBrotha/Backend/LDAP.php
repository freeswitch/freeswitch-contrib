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

require_once('Net/LDAP2.php');
/**
 * @author Mathieu Parent
 */

class MediaBrotha_Backend_LDAP extends MediaBrotha_Backend {
	private $_ldap;
	public function __construct(array $args = Array()) {
		parent::__construct($args);
		$this->_ldap = Net_LDAP2::connect($this->getParam('ldap_connect_config'));
		if (Net_LDAP2::isError($this->_ldap)) {
			die('Could not fetch entry: '.$this->_ldap->getMessage());
		}
	}
	// Helper functions
	public function isURISafe($uri) {
		return true;
	}

	public static function parseLDAPURI($uri, $component = NULL) {
		$uri = preg_replace('@^ldap:///@', 'ldap://fake_host/', $uri);
		$components = parse_url($uri, $component);
		if (is_array($components) && ($components['host'] === 'fake_host')) {
			$components['host'] = NULL;
		}
		return $components;
	}

	// Browsing
	public function mediaFromBufferItem($entry) {
		if ($entry) {
			$media = new MediaBrotha_Media(
				'ldap:///'.$entry->dn(),
				Array(
					'display_name' => $entry->dn(), //$entry->getValue('sn', 'single'),
				),
				'text/directory'
				//$finfo->file($file->getPathname(), FILEINFO_MIME_ENCODING);
			);
			return $media;
		}
	}

	public function fetch(MediaBrotha_Media $media) {
		$uri = $media->getURI();
		if (($this::parseLDAPURI($uri, PHP_URL_SCHEME) != 'ldap') || !$this->isURISafe($uri)) {
			$ldap_connect_config = $this->getParam('ldap_connect_config');
			$uri = 'ldap:///'.$ldap_connect_config['basedn'];
		}
		$basedn = $this::parseLDAPURI($uri, PHP_URL_PATH);
		if ($basedn && ($basedn{0} = '/')) {
			$basedn = substr($basedn, 1);
		}
		$ldap_search = $this->_ldap->search($basedn, NULL,
			Array());
		return new MediaBrotha_MediaIterator($this, $media, $ldap_search);
		return true;
	}
	// Actions
	protected function _isHandled(MediaBrotha_Media $media) {
		return ($media->getMimeType() === 'text/directory')
			|| (substr($media->getURI(), 0, 7) === 'ldap://');
	}

	public function getMediaActions(MediaBrotha_Media $media) {
		if ($this->_isHandled($media)) {
			return Array(
				'browse',
			);
		} else {
			return parent::getMediaActions($media);
		}
	}

}

