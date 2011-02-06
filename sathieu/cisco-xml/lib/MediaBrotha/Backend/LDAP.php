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

	// Actions
	protected function _isHandled(MediaBrotha_Media $media) {
		return ($media->getMimeType() === 'text/directory')
			|| (substr($media->getURI(), 0, 7) === 'ldap://');
	}

	public function getMediaActions(MediaBrotha_Media $media) {
		if ($this->_isHandled($media)) {
			return Array(
				'_default',
				'browse',
				'search',
			);
		} else {
			return parent::getMediaActions($media);
		}
	}
	public function doMediaAction($action, MediaBrotha_Media $media) {
		switch($action) {
			case 'browse':
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
				return new MediaBrotha_MediaIterator($this, $ldap_search);
			case '_default':
			case 'search':
				$form =  new MediaBrotha_Form();
				$form->setTitle('search');
				$search_fields = Array(
					'sn'              => Array('display_name' => 'last name'),
					'givenName'       => Array('display_name' => 'first name'),
					'telephoneNumber' => Array('display_name' => 'phone', 'filter' => '(telephoneNumber=*%s*)'),
					//'o'               => Array('display_name' => 'organization'),
					//'mobile'          => Array('display_name' => 'mobile'),
					//'facsimileTelephoneNumber' => Array('display_name' => 'fax'),
					//'l'               => Array('display_name' => 'location'),
					//'st'              => Array('display_name' => 'department'),
					//'roomNumber'      => Array('display_name' => 'room number'),
				);
				$ldap_connect_config = $this->getParam('ldap_connect_config');
				$filter = $ldap_connect_config['filter'];
				foreach ($search_fields as $attribute_name => $field) {
					$field['name'] = $attribute_name;
					if (!empty($_GET[$attribute_name])) {
						$field['value'] = $_GET[$attribute_name];
						if(empty($infos['filter'])) {
							$filter="(&$filter($attribute_name=".$_GET[$attribute_name]."*))";
						} else {
							$filter="(&$filter".sprintf($infos['filter'], $_GET[$attribute_name]).")";
						}
					}
					$form->addField($field);
				}
				if (empty($_GET['ldap_search'])) {
					MediaBrotha_Core::addCallback('MediaBrotha_Backend_LDAP::searchCallback');
					return $form;
				} else {
					$ldap_search = $this->_ldap->search(NULL, $filter,
						Array('scope' => 'sub'));
					return new MediaBrotha_MediaIterator($this, $ldap_search);
				}
			default:
				return parent::doMediaAction($action, $media);
		}
	}
	static public function searchCallback() {
		$_GET['backend'] = 'LDAP';
		$_GET['action'] = 'search';
		$_GET['ldap_search'] = '1';
	}
}

