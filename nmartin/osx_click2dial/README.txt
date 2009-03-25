README

Client Side Click2Dial

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

The original author of this work, Nik Martin, must be credited  
as the original author of this work.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.


==============================================================

Setup:

Unzip Click2Dial.zip.

Inside fs_dial.app, edit Contents/Resources/fsdial.py to set up your server information and phone number

Import linkify.user.js into greasemonkey (http://www.greasespot.net/)

Double Click fs_dial.app to register it as the handler for dial:// urls

The greasemonkey script will linkify any text on a web page that looks like a phone number.  
You can edit the reg-ex inside the greasemonkey script to match different types of numbers.

The fsdial.py script assumes you have a default context, and outbound calls should go to this context