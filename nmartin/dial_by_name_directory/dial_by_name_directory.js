/**
=======================
Mostly From the Original Code aadir.js:
Original Code Author:  John Wehle           
Date: November 6, 2008
Copyright (c)  2008  Feith Systems and Software, Inc.
All Rights Reserved
=======================


If the prior author's claimed copyright is infringed by the one I claim we'll 
work it out, BUT I prefer a much looser MIT license:
==============================================================

Copyright (c) 2009 Nik Martin

Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without
restriction, including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

The original authors of this work, John Wehle, then Nik Martin, must be credited  
as the original authors of this work.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.


==============================================================
*/

var digitTimeOut = 3000;
var interDigitTimeOut = 1000;
var absoluteTimeOut = 10000;
var base_dir = session.getVariable ("base_dir");
var domain = session.getVariable ("domain");
var voicemail_path = base_dir + "/storage/voicemail/default/" + domain + "/";
var file_exts = [ ".wav", ".mp3" ];
var extRE = /^1[0-9][0-9][0-9]$/g;
var operator = "operator";
var directory;

var translations = [ "0",
					"QZ", "ABC", "DEF",
					"GHI", "JKL", "MNO",
					"PQRS", "TUV", "WXYZ" ];
var extension = "";
var dtmf_digits = "";


/**
 * Build an array of [last_namefirst_name,extension].
 * matches will then start at last name and go until
 * a unique match emerges:
 * martinnik and martinmatt match until the a in matt and i in nik
 * 
 * @return      the image at the specified URL
 */

function build_directory_list() {
	var i;
	var name;
	var number;
	var dir = apiExecute ("xml_locate", "directory domain name " + domain);
	var re = /\s+$/g;
	var length = dir.search (re);
	if (length == -1)
	        length = dir.length;
	dir = dir.substring (0, length);
	var xdir = new XML (dir);
	directory = new Array ();
	i = 0;
	re = /[^A-Z0-9\s]/gi;
	for each (var variables in xdir.groups.group.users.user.variables) {
        
		name = "";
		number = "";
		for each (variable in variables.variable) {
			if (variable.@name.toString() == "effective_caller_id_name")
					  name = variable.@value.toString();
					  
					  //clean up the name string, letters and numbers only
					  name = name.replace (re, "");
			if (variable.@name.toString() == "effective_caller_id_number")
					  number = variable.@value.toString();
		}
		if (name.length == 0 || number.length == 0 || number.search (extRE) == -1)
				continue;
		directory[i] = new Array (2);
		//make the name look like LastnameFirstname
		name = name.split(" ")[1] + name.split(" ")[0];
		directory[i][0] = name;
		directory[i][1] = number;
        console_log ("debug", directory[i][0] + ":" + directory[i][1] + "\n");
		i++;
	}
}

function directory_lookup (digits) {
	var i;
	var match = "";
	var pattern = "^";
	var re;
	if (digits.length && digits[0] == 0)
		 return 0;
	for (i = 0; i < digits.length; i++) {
		if (isNaN (parseInt (digits[i], 10)))
			return -1;
		//build a regex pattern to match against
		pattern += "[" + translations[parseInt (digits[i], 10)] + "]";
		console_log("debug", "pattern: " + pattern + "\n");
	}
	re = new RegExp (pattern, "i");
	for (i = 0; i < directory.length; i++)
		 if (directory[i][0].search (re) != -1) {
		if (! isNaN (parseInt (match, 10)))
				  return "";
		match = directory[i][1];
	}
	if (isNaN (parseInt (match, 10)))
		 return -1;
	return match;
}
function on_dtmf (session, type, obj, arg) {
	if (type == "dtmf") {
		dtmf_digits += obj.digit;
		extension = directory_lookup (dtmf_digits)
		return false;
	}
	return true;
}
function directory_prompt () {
	var choice;
	var index;
	var repeat;
	extension = "";
	choice = "";
	repeat = 0;
	while (session.ready() && repeat < 3) {
		/* play phrase - if digit keyed while playing callback will catch them*/
		session.sayPhrase ("dial_by_name_directory", "#", "", on_dtmf, "");
		choice = dtmf_digits;
		while ( isNaN (parseInt (extension, 10)) ) {
			if (! session.ready ())
					  return "";
			dtmf_digits = session.getDigits (1, '#', digitTimeOut,
											interDigitTimeOut, absoluteTimeOut);
			choice += dtmf_digits;
			console_log ("debug", "choice: "  + choice + "\n");
			extension = directory_lookup(choice);
		}
		if (parseInt (extension, 10) >= 0)
				break;
		session.sayPhrase ("voicemail_invalid_extension", "#", "", on_dtmf, "");
		dtmf_digits = "";
		extension = "";
		choice = "";
		repeat++;
		session.flushDigits ();
	}
	return extension;
}
var choice = "";
var fd;
var i;
var recorded_name;
session.answer ();
session.execute("sleep", "1000");
console_log("info", "loading dial-by-name directory\n");
build_directory_list();
dtmf_digits = "";
session.flushDigits ();
choice = directory_prompt ();

if (! session.ready ()) {
	session.hangup();
	exit();
}

//this must try to determine if the extension is a number, but it seems that
//non-integer extension are ok in FS.  Leftover from original code
if ( isNaN (parseInt (choice, 10)) || parseInt (choice, 10) <= 0) {
	session.execute ("transfer", operator + " XML default");
	exit();
}

for (i = 0; i < file_exts.length; i++) {
	recorded_name = voicemail_path + choice + "/recorded_name" + file_exts[i];
	fd = new File (recorded_name);
	if (fd.exists) {
		session.streamFile (recorded_name);
		break;
	}
//TODO: say <module_name> <say_type> <say_method> <text> << spell out the name if they have not recorded one yet

}


session.execute ("phrase", "spell," + choice);
//TODO: prompt to see if they REALLY want this person, then go back up if not.

session.execute ("transfer", choice + " XML features");
exit();