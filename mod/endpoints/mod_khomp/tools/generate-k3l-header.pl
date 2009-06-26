#!/usr/bin/perl

my @files =
    ( 'KTypeDefs.h', 'KVoIP.h', 'KDefs.h', 'KErrorDefs.h', 'KH100Defs.h',
      'KMixerDefs.h', 'KR2D.h', 'KISDN.h', 'KGSM.h', 'KStats.h', 'k3lVersion.h', 'k3l.h' );

my $text = "";

foreach my $file (@files)
{
    open FILE, ('api/' . $file);

    while (<FILE>)
    {
        $text .= $_;
    }

    close (FILE);
}

$text =~ s/[\/][\*](.+?)[\*][\/]//smg;
$text =~ s/[\/][\/](.+)//mg;
$text =~ s/\#include.+\n//g;
$text =~ s/\n\n/\n/mg;

my $header =
"/*  
    KHOMP endpoint provider for FreeSWITCH(tm)
    Copyright (C) 2007-2009 Khomp Ind. & Com.  
  
  The contents of this file are subject to the Mozilla Public License Version 1.1
  (the \"License\"); you may not use this file except in compliance with the
  License. You may obtain a copy of the License at http://www.mozilla.org/MPL/

  Software distributed under the License is distributed on an \"AS IS\" basis,
  WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License for
  the specific language governing rights and limitations under the License.

  Alternatively, the contents of this file may be used under the terms of the
  \"GNU Lesser General Public License 2.1\" license (the “LGPL\" License), in which
  case the provisions of \"LGPL License\" are applicable instead of those above.

  If you wish to allow use of your version of this file only under the terms of
  the LGPL License and not to allow others to use your version of this file under
  the MPL, indicate your decision by deleting the provisions above and replace them
  with the notice and other provisions required by the LGPL License. If you do not
  delete the provisions above, a recipient may use your version of this file under
  either the MPL or the LGPL License.

  The LGPL header follows below:

    This library is free software; you can redistribute it and/or  
    modify it under the terms of the GNU Lesser General Public  
    License as published by the Free Software Foundation; either  
    version 2.1 of the License, or (at your option) any later version.  
  
    This library is distributed in the hope that it will be useful,  
    but WITHOUT ANY WARRANTY; without even the implied warranty of  
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU  
    Lesser General Public License for more details.  
  
    You should have received a copy of the GNU Lesser General Public License
    along with this library; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA 
  
*/

";

my $footer =
"
enum KISDNDebugFlag
{
    kidfQ931                        = 0x01,
    kidfLAPD                        = 0x02,
    kidfSystem                      = 0x04,
    kidfInvalid                     = 0x08,
};

#define CM_VOIP_START_DEBUG      0x20
#define CM_VOIP_STOP_DEBUG       0x21
#define CM_VOIP_DUMP_STAT        0x22

#define CM_ISDN_DEBUG            0x24

#define CM_PING     0x123456
#define EV_PONG     0x654321

#define CM_LOG_REQUEST                  0x100
#define CM_LOG_CREATE_DISPATCHER        0x101
#define CM_LOG_DESTROY_DISPATCHER       0x102

";

open FILE, '>', '../include/k3lDefs.h';
print FILE $header;
print FILE "#ifndef INCLUDED_K3LDEFS_H\n";
print FILE "#define INCLUDED_K3LDEFS_H\n";
print FILE $text;
print FILE $footer;
print FILE "#endif /* INCLUDED_K3LDEFS_H */\n";
close FILE;
