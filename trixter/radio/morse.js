//  -*- mode:c; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil;  -*-
// Copyright (C) 2009, Bret McDanel <trixter AT 0xdecafbad.com>
//
// Version: MPL 1.1 
//
// The contents of this file are subject to the Mozilla Public License Version 
// 1.1 (the "License"); you may not use this file except in compliance with 
// the License. You may obtain a copy of the License at 
// http://www.mozilla.org/MPL/
//
// Software distributed under the License is distributed on an "AS IS" basis,
// WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License 
// for the specific language governing rights and limitations under the 
// License.
//
// The Original Code is Morse code generation jsapi
//
// The Initial Developer of the Original Code is 
// Bret McDanel <trixter AT 0xdecafbad.com>
// Portions created by the Initial Developer are Copyright (C)
// the Initial Developer. All Rights Reserved.
//
// Contributor(s):
//
// Bret McDanel <trixter AT 0xdecafbad.com>
//
// Morse code generation jsapi
// This will play morse code to a given target whatever string is 
// supplied
//
// examples:
//    jsapi conference 123 morse code is great
//    jsapi uuid b0b05504-97be-11de-a116-611fa158f341 morse code is great

var cw_wpm=20;
var cw_freq=900; // in Hz
var cw_vol=-15;  // -63.0dB to 0.0dB


// you probably do not need to edit anything below here
var cw_unit=parseInt(1000/cw_wpm);

var DIT = "%("+cw_unit+","+cw_unit+","+cw_freq+");";
var DAH = "%("+cw_unit*3+","+cw_unit*3+","+cw_freq+");";
                                                
// so far there are not any prosigns which are just made up from
// the letters below, but the spacing needs to be adjusted
// I also do not have the non-officially defined characters
var charCodes = new Array();
charCodes["a"]="._";
charCodes["b"]="_...";
charCodes["c"]="_._.";
charCodes["d"]="_..";
charCodes["e"]=".";
charCodes["f"]=".._.";
charCodes["g"]="__.";
charCodes["h"]="....";
charCodes["i"]="..";
charCodes["j"]=".___";
charCodes["k"]="_._";
charCodes["l"]="._..";
charCodes["m"]="__";
charCodes["n"]="_.";
charCodes["o"]="___";
charCodes["p"]=".__.";
charCodes["q"]="__._";
charCodes["r"]="._.";
charCodes["s"]="...";
charCodes["t"]="_";
charCodes["u"]=".._";
charCodes["v"]="..._";
charCodes["w"]=".__";
charCodes["x"]="_.._";
charCodes["y"]="_.__";
charCodes["z"]="__..";
charCodes["1"]=".____";
charCodes["2"]="..___";
charCodes["3"]="...__";
charCodes["4"]="...._";
charCodes["5"]=".....";
charCodes["6"]="_....";
charCodes["7"]="__...";
charCodes["8"]="___..";
charCodes["9"]="____.";
charCodes["0"]="_____";
charCodes["/"]="_.._.";

function usage()
{
    console_log("ERROR","Usage: jsapi morse.js [conference|uuid] [target] <string>\n");
    exit();
}


if(typeof argv[2] == "undefined" || argv[2].length==0) {
    usage();
 }


if(argv[0] == "conference") {
    command="conference";
    argv.shift();
    target=argv[0]+" play ";
    argv.shift();
 } else if(argv[0] == "uuid") {
    command="uuid_broadcast";
    argv.shift();
    target=argv[0]+" ";
    argv.shift();
 } else {
    usage();
 }


while(typeof argv[0] != "undefined") {
    var newStr="tone_stream://v="+cw_vol+";";
    for(var i=0;i<argv[0].length;i++) {
        for(var j=0;j<charCodes[argv[0][i]].length;j++) {
            if(charCodes[argv[0][i]][j] == ".") {
                newStr+=DIT;
            } else {
                newStr+=DAH;
            }
        }
        // symbol space
        newStr+="%(0,"+cw_unit*3+","+cw_freq+")";
    }
    // word space
    // we only do a cw_unit*4 here because we have a cw_unit*3 already
    newStr+="%(0,"+cw_unit*4+","+cw_freq+")";
    apiExecute(command, target+newStr);
    argv.shift();
 }
