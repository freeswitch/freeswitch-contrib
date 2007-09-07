//  -*- mode:c; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil;  -*-
// DISA (Direct Inward System Access) script
// basically it provides a dialtone for callers, who can then call out
// on presumably a cheaper rate plan, in addition providing a unified
// caller id presence for return calls
//
// Written by Bret McDanel <trixter AT 0xdecafbad.com>
//
// Additions/Updates by
//
//
// This file is licensed under the Trixter General License (TGL) v 1.0
// You may use this program however you want, so long as you follow the
// below terms:
// 1. You may not make this program GNU Public Licensed (GPL)
// 2. Copyright notices must remain intact
// 3. You acknowledge this program is useful for nothing, and that if you
//    do use it in any way any authors, contributing or original are not
//    held liable for any losses or damage that results from using it
// 4. Terms 1 and 2 may not be altered and must be distributed in future copies
// 5. At your option you may remove condition 3 providing that you assume responsibility
//    from that point on


// START OF CONFIGURATION SECTION

var pinmax    = 10;   // maximum length of a pin - 0 disables pins
var pinwait   = 5000; // ms to wait for pin entry
var pinterm   = "#";  // terminator char for pin entry - cant be part of the pin number
var pinmaxtry = 3;    // maximum number of times to ask for a pin before giving up

var nummaxlen = 11;   // maximum length of a dialed numner
var numterm   = "#";  // terminator for destination number
var numwait   = 5000; // ms for them to enter the destination number

var dialprefix = "sofia/default/"; // prefix for outbound calls
var timeout    = 60;               // outbound calling timeout in seconds


var CLID_name = "The Omega Corporation"; // will use inbound caller's info if commented
                                         // if you dont know who this is, watch more
                                         // monty python
var CLID_num  = "18005551212";           // will use inbound caller's info if commented

// Sound files that are required - if they are commented they will be skipped
var snd_prefix      = "/sounds";         // sound directory prefix
var snd_getpin      = "get-pin.raw";     // prompt asking for their pin number
                                         // cannot be commented - disable by seeing pinmax=0
var snd_blacklisted = "blacklisted.raw"; // inform them to go away
var snd_pinfail     = "bad-pin.raw";     // sound file to play when they enter a bad pin
var snd_pinbye      = "goodbye.raw";     // when they exhausted pin attempts and didnt auth
var snd_getdest     = "enternum.raw";    // "enter the telephone number you wish to call"


// this is the regexps of caller id information that is blacklisted
// dont forget that Javascript/ECMAScript requires you to escape \ so use \\
// in regexps
blacklisted = new Array (new RegExp("^1900\\d{7}$"), // US/CA premium numbers
                         new RegExp("^1710\\d{7}$")  // US Government DARPA network
                         );
                         
                         
// this is the regexps of automatically authenticated callers
whitelisted = new Array (new RegExp("^1337$") // they are elite!
                         );

// Array of PINs for access, in this case an account number would amount to a
// 'secondary pin' although most likely more well known.  This information could
// be obtained from a DB or HTTP resource instead, but in a simple example that is
// overkill - fetchURLHash for example returns an array so it makes it easy to replace
pins = new Array ("1234", // Alice
                  "5678"  // Bob
                  );




// END OF CONFIGURATION SECTION


                                                  
// This function matches session.caller_id_num against an array of RegExps that is passed
// and returns a boolean determining if there was a match, useful for whitelist/blacklisting
function CIDMatch(regexps)
{
    if(typeof regexps != "undefined") {
        for (var i=0; i<regexps.length; i++) {
            if(arrMatch = regexps[i].exec(session.caller_id_number)) {
                return true;
            }
        }
        return false;
    }
}


// this function strips a listed character out of a string
function removeChr(str,chr)
{
    var newStr = "";

    for(var i=0;i<str.length;i++) {
        if(str.substr(i,1) != chr) { newStr += str.substr(i,1); }
    }

    return newStr;
}


// this is the dtmf callback function - in a nice generic way
function main_cb(type,digits,args)
{
    if(type == "dtmf") {
        if(digits!='false') { return digits; }
        return false; // stop playback if any
    }
}





// MAIN PROGRAM ENTRY POINT

if(CIDMatch(blacklisted)) {
    if(typeof snd_blacklisted != "undefined") {
        session.streamFile(snd_blacklsited);
    }
    console_log(session.caller_id_num + " is blacklisted\n");
    exit();
}

session.answer();

if(!CIDMatch(whitelisted)) {
    // Get a pin number
    if(pinmax > 0) {
        var isAuthed=false;
        for(var curpintry=0; isAuthed == false && curpintry < pinmaxtry; curpintry++) {
            session.flushDigits(); // clear out input buffers
            if((pin=session.streamFile(snd_getpin,main_cb,""))==false) {
                pin=session.getDigits(pinmax,pinterm,pinwait);
            } else {
                pin+=session.getDigits(pinmax-1,pinterm,pinwait);
            }

            pin = removeChr(pin,pinterm); // make sure the didnt enter it during the prompt

            if(pin != "" && pin != "false") {
                for (var i=0;i<pins.length && isAuthed==false;i++) {
                    if(pins[i] == pin) {
                        console_log("checking pin against " + pins[i] + "\n");
                        isAuthed=true;
                    }
                }
            }

            if(isAuthed == false) {
                if(typeof snd_pinfail != "undefined") {
                    session.streamFile(snd_pinfail);
                }
            }
            
        }

        if(isAuthed == false) {
            if(typeof snd_pinbye != "undefined") {
                session.streamFile(snd_pinbye);
            }
            console_log(session.caller_id_num + " failed to authenticate\n");
            exit();
        }
    }
}


// there should probably be better error checking here
// if the number doesnt work ask to get a new one
// if the rmeote party hangs up possibly allow the caller to call someone else
// but I got bored with this and decided it was good enough for now

// Get destination number
session.flushDigits();
if((destnum=session.streamFile(snd_getdest,main_cb,""))==false) {
    destnum=session.getDigits(nummaxlen,numterm,numwait);
} else {
    destnum+=session.getDigits(nummaxlen-1,numterm,numwait);
}
destnum = removeChr(destnum,numterm);

if(destnum.length>0) {
    if(typeof CLID_name != "undefined") {
        session.setVariable("effective_caller_id_name",CLID_name);
    }
    if(typeof CLID_num != "undefined") {
        session.setVariable("effective_caller_id_num",CLID_num);
    }

    session.originate("",dialprefix+destnum,timeout);
}

exit();
