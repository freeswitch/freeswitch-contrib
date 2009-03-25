// FreeSWITCH Dial Linkify
// Author: Nik Martin 
// License: MIT
// 
//This script was taken from another script here: http://userscripts.org/scripts/show/1003
//
// Matches these patterns:
//  800-555-1212
//  (800) 555-1212
//  (800)555-1212
//  8005551212
//    Weak since it does any 10+ digit number
//  800.555.1212
// Tested with:
//  www.yellowpages.com
//  local.google.com
//  a9.com
//
// International calling sure would be neat
// ==UserScript==
// @name           FreeSWITCH Dial Linkify
// @namespace      
// @description    Looks for phone numbers in pages and makes hyperlinks out of them. When clicking on the link, FS will connect your phone to the number you clicked
// @include        *
// ==/UserScript==

(function () {
	
//this is the test to see if the number looks like a phone number or not
	const trackRegex = /(\+\d\d?)?[\-\s\/\.]?[\(]?(\d){2,4}[\)]?[\-\s\/\.]?\d\d\d[\-\s\/\.]?(\d){3,5}\b/ig;

	function trackUrl(t) {
		return "dial://org.FreeSWITCH.Dialer?number=" + String(t).replace(/[\(\)\- \.]/g, "");
	}

    // tags we will scan looking for un-hyperlinked urls
    var allowedParents = [
        "abbr", "acronym", "address", "applet", "b", "bdo", "big", "blockquote", "body", 
        "caption", "center", "cite", "code", "dd", "del", "div", "dfn", "dt", "em", 
        "fieldset", "font", "form", "h1", "h2", "h3", "h4", "h5", "h6", "i", "iframe",
        "ins", "kdb", "li", "object", "nobr", "pre", "p", "q", "samp", "small", "span", "strike", 
        "s", "strong", "sub", "sup", "td", "th", "tt", "u", "var"
        ];
    
    var xpath = "//text()[(parent::" + allowedParents.join(" or parent::") + ")" +
				//" and contains(translate(., 'HTTP', 'http'), 'http') + " + 
				"]";

    var candidates = document.evaluate(xpath, document, null, XPathResult.UNORDERED_NODE_SNAPSHOT_TYPE, null);

    //var t0 = new Date().getTime();
    for (var cand = null, i = 0; (cand = candidates.snapshotItem(i)); i++) {
        if (trackRegex.test(cand.nodeValue)) {
            var span = document.createElement("span");
            var source = cand.nodeValue;
            
            cand.parentNode.replaceChild(span, cand);

            trackRegex.lastIndex = 0;
            for (var match = null, lastLastIndex = 0; (match = trackRegex.exec(source)); ) {
                span.appendChild(document.createTextNode(source.substring(lastLastIndex, match.index)));
                
                var a = document.createElement("a");
                a.setAttribute("href", trackUrl(match[0]));
                a.appendChild(document.createTextNode(match[0]));
                span.appendChild(a);

                lastLastIndex = trackRegex.lastIndex;
            }

            span.appendChild(document.createTextNode(source.substring(lastLastIndex)));
            span.normalize();
        }
    }
   // var t1 = new Date().getTime();
   // alert("X-Lite Dial linkify took " + ((t1 - t0) / 1000) + " seconds");

})();
