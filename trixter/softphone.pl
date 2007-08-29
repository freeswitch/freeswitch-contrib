#!/usr/bin/perl
# -*- mode:cperl; tab-width:4; c-basic-offset:4; c-indent-level:4; indent-tabs-mode:nil;  -*-
# 
# FreeSWITCH Softphone
# Copyright (C) 2007, Bret McDanel <trixter AT 0xdecafbad.com>
#
# Version: MPL 1.1
#
# The contents of this file are subject to the Mozilla Public License Version
# 1.1 (the "License"); you may not use this file except in compliance with
# the License. You may obtain a copy of the License at
# http://www.mozilla.org/MPL/
#
# Software distributed under the License is distributed on an "AS IS" basis,
# WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
# for the specific language governing rights and limitations under the
# License.
#
# The Original Code is FreeSWITCH Softphone
#
# The Initial Developer of the Original Code is
# Bret McDanel <trixter AT 0xdecafbad.com>
# Portions created by the Initial Developer are Copyright (C)
# the Initial Developer. All Rights Reserved.
#
# Contributor(s):
# 
# Bret McDanel <trixter AT 0xdecafbad.com>
#
# softphone.pl -- FreeSWITCH softphone in perl
#
#

#####
##
## This is the start of a GUI for FreeSWITCH[tm] using port audio as the channel endpoint.
## Right now there are a few things that do not work right:
##   * events are not properly read
##   * if remote party hangs up the client doesnt realize it
##   * registration status is not displayed
##   * no redial ability
##   * no transfer, hold, etc
##   * incoming calls cannot be detected, or answered
##
## It works for a single channel, and you can type anything into the dial window, if that matches some regexp
## in your dialplan the call will proceed.
##
#####

use FreeSWITCH::Client;
use Data::Dumper; # used to print out myhash debug info
use Tk;
use strict;


# configure these
my $password = "ClueCon";                    # the password for event socket
my $host     = "localhost";                  # the hostname to connect to
my $port     = 8021;                         # the port to connect to


# random variables that we are going to use
my $window;
my $phonenumber;
my $callInProgress=0;
my $fs;
my $errorString = undef;


makeWindow();
MainLoop;
$fs->sendmsg({'command' => "api pa hangup"}) unless !defined($fs);
print "We should hang up the calls, and any other cleanup here\n";



sub getEvent() {
  if(defined $fs) {
    my $reply = $fs->readhash(0);
    if ($reply->{socketerror}) {
      $errorString = "Got error: ".$reply->{socketerror};
      es_connect();
    }
    
    if ($reply->{body}) {
      my $myhash = $reply->{event};
      print Dumper $myhash;
    } 
  } else {
    $errorString = "Connecting";
    es_connect();
  }
}


# this connects to the event socket
sub es_connect()
{
    $errorString = "Connecting to $host:$port";
    eval {
      $fs = init FreeSWITCH::Client {-password => $password, -host => $host, -port => $port};
      if(defined $fs) {
        $fs->sendmsg({'command' => 'event plain myevents'});
      }
    } or do {
      $errorString = "Error connecting - waiting for retry";
      sleep 5;
    }
    
}


sub makeWindow {
    $window = MainWindow->new();
    $window->title("FreeSWITCH[tm] GUI");
    my $mainframe = $window->Frame()->pack(qw/-side top -padx 10 -expand true -fill both/);
    
    $mainframe->Entry(-textvariable => \$phonenumber)->pack(qw/-side top/);
    my $dialpadframe = $mainframe->Frame()->pack(qw/-anchor s -pady 3/);
    my $dialpadcol1 = $dialpadframe->Frame()->pack(qw/-side left -pady 3/);
    my $dialpadcol2 = $dialpadframe->Frame()->pack(qw/-side left -pady 3/);
    my $dialpadcol3 = $dialpadframe->Frame()->pack(qw/-side left -pady 3/);
    my $errorMsgs = $mainframe->Label(-textvariable => \$errorString)->pack(qw/-side bottom/);
    my $buttonframe = $mainframe->Frame()->pack(qw/-side bottom/);

    # I really dont like this method, but I dont know Tk that well
    # basically we have a timer that will call getEvent() 1ms after its finished running
    # to see about getting more events - threads dont work because you cant share the FreeSWITCH::Client class
    # forking doesnt work because $fs is always undefined from the GUI side
    # or maybe you can, I dont know perl all that well either.
    my $timerid = $window->repeat(100,\&getEvent);
    
# set up the keys
    my $phonebutton1     = $dialpadcol1->Button(-text => '1', -command => sub{sendDTMF("1")})->pack(qw/-side top/);
    my $phonebutton2     = $dialpadcol2->Button(-text => '2', -command => sub{sendDTMF("2")})->pack(qw/-side top/);
    my $phonebutton3     = $dialpadcol3->Button(-text => '3', -command => sub{sendDTMF("3")})->pack(qw/-side top/);
    my $phonebutton4     = $dialpadcol1->Button(-text => '4', -command => sub{sendDTMF("4")})->pack(qw/-side top/);
    my $phonebutton5     = $dialpadcol2->Button(-text => '5', -command => sub{sendDTMF("5")})->pack(qw/-side top/);
    my $phonebutton6     = $dialpadcol3->Button(-text => '6', -command => sub{sendDTMF("6")})->pack(qw/-side top/);
    my $phonebutton7     = $dialpadcol1->Button(-text => '7', -command => sub{sendDTMF("7")})->pack(qw/-side top/);
    my $phonebutton8     = $dialpadcol2->Button(-text => '8', -command => sub{sendDTMF("8")})->pack(qw/-side top/);
    my $phonebutton9     = $dialpadcol3->Button(-text => '9', -command => sub{sendDTMF("9")})->pack(qw/-side top/);
    my $phonebuttonstar  = $dialpadcol1->Button(-text => '*', -command => sub{sendDTMF("*")})->pack(qw/-side top/);
    my $phonebutton0     = $dialpadcol2->Button(-text => '0', -command => sub{sendDTMF("0")})->pack(qw/-side top/);
    my $phonebuttonpound = $dialpadcol3->Button(-text => '#', -command => sub{sendDTMF("#")})->pack(qw/-side top/);
    
    
    my $dialbutton = $buttonframe->Button(-text => 'Dial/Answer', -command => \&dialAnswer)->pack(qw/-side left/);
    my $hangupbutton = $buttonframe->Button(-text => 'Hangup', -command => \&hangup)->pack(qw/-side right/);
}    

sub dialAnswer {
    $callInProgress=1;
    if(defined($fs)) {
      $fs->sendmsg({'command' => "api pa call $phonenumber"});
    } else {
      $errorString = "Not connected!!!";
    }
}

sub hangup {
    $callInProgress=0;
    $errorString = "hangup";
    $phonenumber = "";
    $fs->sendmsg({'command' => "api pa hangup"}) unless !defined($fs);
}

sub sendDTMF{
    my $dtmf=shift;
    if($callInProgress==1) {
	if(defined($fs)) {
	    $fs->sendmsg({'command' => "api pa dtmf $dtmf"}) unless !defined($dtmf);
	} else {
	    $errorString = "Not connected!";
	}
    } else {
	$phonenumber .= $dtmf unless !defined($dtmf);
    }
}
