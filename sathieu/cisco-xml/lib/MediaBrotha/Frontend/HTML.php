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
			foreach ($backend->getVisibleMediaActions($media) as $action) {
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

	private function _createHTMLDocument($title) {
		$this->_xml = new DOMDocument('1.0');
		$this->_xml->formatOutput = true;
		$root = $this->_xml->createElement('html');
		$root = $this->_xml->appendChild($root);

		$head = $this->_xml->createElement('head');
		$head = $root->appendChild($head);

		$titleElement = $this->_xml->createElement('title');
		$titleElement = $head->appendChild($titleElement);
		$text = $this->_xml->createTextNode($title);
		$text = $titleElement->appendChild($text);

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
	}

	public function begin($item) {
		$this->_createHTMLDocument($item->getDisplayName());
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

	public function renderForm(MediaBrotha_Form $form) {
		$this->_createHTMLDocument($form->getTitle());
		$formElement = $this->_xml->createElement('form');
		$formElement = $this->_body->appendChild($formElement);

		$tableElement = $this->_xml->createElement('table');
		$tableElement = $formElement->appendChild($tableElement);

		foreach($form as $field) {
			if ($field->get('visibility') === 'hidden') {
				$inputElement = $this->_xml->createElement('input');
				$inputElement = $formElement->appendChild($inputElement);
				$inputElement->setAttribute('name', $field->get('name'));
				$inputElement->setAttribute('value', $field->get('value'));
				$inputElement->setAttribute('type', 'hidden');
			} else {
				$trElement = $this->_xml->createElement('tr');
				$trElement = $tableElement->appendChild($trElement);

				$tdElement1 = $this->_xml->createElement('th');
				$tdElement1 = $trElement->appendChild($tdElement1);
				$text1 = $this->_xml->createTextNode($field->get('display_name'));
				$text1 = $tdElement1->appendChild($text1);

				$tdElement2 = $this->_xml->createElement('td');
				$tdElement2 = $trElement->appendChild($tdElement2);

				$inputElement = $this->_xml->createElement('input');
				$inputElement = $tdElement2->appendChild($inputElement);
				$inputElement->setAttribute('name', $field->get('name'));
				$inputElement->setAttribute('value', $field->get('value'));
			}
		}

		$trElement = $this->_xml->createElement('tr');
		$trElement = $tableElement->appendChild($trElement);

		$tdElement = $this->_xml->createElement('td');
		$tdElement = $trElement->appendChild($tdElement);
		$tdElement->setAttribute('colspan', 2);

		$inputElement = $this->_xml->createElement('input');
		$inputElement = $tdElement->appendChild($inputElement);
		$inputElement->setAttribute('type', 'submit');

		print $this->_xml->saveHTML();
	}
	public function renderException(Exception $e) {
		$this->_createHTMLDocument('Error');
		$divElement = $this->_xml->createElement('pre');
		$divElement = $this->_body->appendChild($divElement);
		$text = $this->_xml->createTextNode($e->getMessage());
		$text = $divElement->appendChild($text);

		$divElement = $this->_xml->createElement('pre');
		$divElement = $this->_body->appendChild($divElement);
		$text = $this->_xml->createTextNode($e->getTraceAsString());
		$text = $divElement->appendChild($text);

		print $this->_xml->saveHTML();
	}

}

