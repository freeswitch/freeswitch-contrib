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

require_once('HTTP.php');

class MediaBrotha_Frontend_HTML extends MediaBrotha_Frontend_HTTP {
	private $_xml = NULL;
	private $_body = NULL;
	private $_mediaListElement = NULL;
	private $_actionsListElement = NULL;

	private function _mediaActionLinks($parent, $media, $tag_name) {
		$url = $this->rootURL().'?'.
			MediaBrotha_Core::value2hash(
				'mime_type='.urlencode($media->getMimeType()).'&'.
				($media->getMimeEncoding() ? ('mime_encoding='.urlencode($media->getMimeEncoding()).'&') : '').
				'uri='.urlencode($media->getURI()));

		foreach (MediaBrotha_Core::getBackends() as $backend) {
			foreach ($backend->getMediaActions($media) as $action) {
				$tag = $this->_xml->createElement($tag_name);
				$tag = $parent->appendChild($tag);

				$a = $this->_xml->createElement('a');
				$a = $tag->appendChild($a);
				$a->setAttribute('href', "$url&".MediaBrotha_Core::value2hash('action='.$action.'&backend='.$backend->getBackendName()));

				$text = $this->_xml->createTextNode($action);
				$text = $a->appendChild($text);
			}
		}
	}

	public function begin($item) {
		$this->_xml = new DOMDocument('1.0');
		$root = $this->_xml->createElement('html');
		$root = $this->_xml->appendChild($root);

		$head = $this->_xml->createElement('head');
		$head = $root->appendChild($head);

		$title = $this->_xml->createElement('title');
		$title = $head->appendChild($title);

		$text = $this->_xml->createTextNode($item->getDisplayName());
		$text = $title->appendChild($text);

		$style = $this->_xml->createElement('link');
		$style = $head->appendChild($style);
		$style->setAttribute('type', 'text/css');
		$style->setAttribute('rel', 'stylesheet');
		$style->setAttribute('href', $this->rootURL().'static/default.css');

		$script = $this->_xml->createElement('script');
		$script = $head->appendChild($script);
		$script->setAttribute('type', 'text/javascript');
		$script->setAttribute('src', $this->rootURL().'static/dynamic.js');

		$this->_body = $this->_xml->createElement('body');
		$this->_body = $root->appendChild($this->_body);
		$this->_body->setAttribute('onload', 'bodyLoaded();');

		$this->_mediaListElement = $this->_xml->createElement('ul');
		$this->_mediaListElement = $this->_body->appendChild($this->_mediaListElement);
		$this->_mediaListElement->setAttribute('id', 'mediaListElement');
		if ($parent = $item->getParent()) {
			$this->addItem($parent);
		}
	}
	public function addItem($item) {
		$url = $this->rootURL().'?'.
			MediaBrotha_Core::value2hash(
				'mime_type='.urlencode($item->getMimeType()).'&'.
				($item->getMimeEncoding() ? ('mime_encoding='.urlencode($item->getMimeEncoding()).'&') : '').
				'uri='.urlencode($item->getURI()));

		$li = $this->_xml->createElement('li');
		$li = $this->_mediaListElement->appendChild($li);
		$li->setAttribute('class', 'mediaElement');

		$a = $this->_xml->createElement('a');
		$a = $li->appendChild($a);
		$a->setAttribute('href', $url);

		$text = $this->_xml->createTextNode($item->getDisplayName());
		$text = $a->appendChild($text);

		$ul = $this->_xml->createElement('ul');
		$ul = $li->appendChild($ul);
		$ul->setAttribute('class', 'mediaActionsListElement');

		$this->_mediaActionLinks($ul, $item, 'li');
	}
	public function finish($item) {
		$this->_actionsListElement = $this->_xml->createElement('ul');
		$this->_actionsListElement = $this->_body->appendChild($this->_actionsListElement);
		$this->_actionsListElement->setAttribute('id', 'actionsListElement');
		$this->_mediaActionLinks($this->_actionsListElement, $item, 'li');
	}
	public function render() {
		print $this->_xml->saveHTML();
	}
}

