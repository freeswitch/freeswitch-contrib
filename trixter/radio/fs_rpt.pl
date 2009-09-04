#!/usr/bin/perl
# -*- mode:perl; tab-width:4; c-basic-offset:4; c-indent-level:4; indent-tabs-mode:nil;  -*-
# Copyright (C) 2009, Bret McDanel <trixter AT 0xdecafbad.com>
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
# The Original Code is FreeSWITCH Radio Interface
#
# The Initial Developer of the Original Code is 
# Bret McDanel <trixter AT 0xdecafbad.com>
# Portions created by the Initial Developer are Copyright (C)
# the Initial Developer. All Rights Reserved.
#
# Contributor(s):
#
# Bret McDanel <trixter AT 0xdecafbad.com>

# WARNING:
#    Certain laws apply in virtually every jurisdiction to the use 
#    of radio transmitters.  By connecting a radio transmitter to
#    your system you assume all responsibility for its use, and the
#    content that others may provide.
#    Be aware of the laws, and follow them.

# This script will connect to event socket and bridge a ham radio into
# a conference via portaudio.  
#
# If anyone on the radio side presses DTMF it can react to that
#    Autopatch
#    Bring up a prespecified link (echolink or irlp type functionality)
#    Announce some information
#    Perform other actions like kicking people out of the conference
#
# If anyone on the internet side connects
#    Their audio is relayed through the radio using the talking events






use Device::SerialPins;
use Getopt::Std;
use strict;
use FreeSWITCH::Client;
use POSIX ':signal_h'; # used for alarm to ensure we get heartbeats 
use Switch;
use Data::Dumper; # used to print out myhash debug info                         
use File::stat;


my $password = "ClueCon";    # event socket password
my $host     = "localhost";  # event socket host
my $port     = 8021;         # event socket port
my $device   = undef;        # radio control device (/dev/ttyS0, COM1, etc)
my $baud     = 9600;         # radio control device baud rate
my $timeout  = 30;           # seconds to expect a heartbeat or reconnect
my $courtesy_tone = "tone_stream://%(150,150,500);%(150,0,400)"; # tone played before releasing PTT
#my $courtesy_tone = "/sounds/beep.wav"; # play a sound file
#my $courtesy_tone = undef; # disable courtesy tone
my $confname = "radio";      # the name of the conference
my $extension = "1337";      # this is the extension that portaudio will call to join
my $callsign = undef;        # disable callsign autoID - set to your callsign
my $callsign_interval = 600; # 10 minute intervals


# for TTS anouncements played after CWID - undef to disable
my $voice = "Allison";
my $swift = "/opt/swift/bin/swift";
my $filetime = 0;


# normal users do not need to edit anything below here
my %options;
my $fs;
my $lastheartbeat;
my $lastcallsign;
my $lasttx;
my $releasePTT=0;
my $ptt_port;



sub pressPTT()
{
    $ptt_port->set_rts(1);
}

sub releasePTT()
{
    $ptt_port->set_rts(0);
}


# this connects to the event socket 
sub es_connect()
{
    print "Connecting to $host:$port\n";
    eval {
        $fs = init FreeSWITCH::Client {-password => $password, -host => $host, -port => $port};
        if(defined $fs) {
            $fs->sendmsg({'command' => 'event plain heartbeat CUSTOM conference::maintenance'});
            $lastheartbeat = time;
        }
    } or do {
        print "Error connecting - waiting for retry\n";
        sleep(10);
    }
}


sigaction SIGALRM, new POSIX::SigAction sub { 
    if ($lastheartbeat < (time - $timeout)) {
        print "Did not receive a heartbeat in the specified timeout\n";
        if (defined $fs) {
            $fs->disconnect();
            undef $fs;
        }
        es_connect();
    }
    
    if(defined $callsign && $lastcallsign < (time - $callsign_interval) && $lasttx > $lastcallsign) {
        pressPTT();
        $fs->command("jsapi morse.js conference radio ".$callsign);
        $lastcallsign = time;
        $releasePTT++;


        if (-f "announcement.txt") {
            if(stat("announcement.txt")->mtime > $filetime && defined $voice $$ defined $swift) {
                system("$swift -p audio/deadair=2000,audio/sampling-rate=8000,audio/channels=1,audio/encoding=pcm16,audio/output-format=raw -o /tmp/announcement.raw -f announcement.txt -n $voice");
                $fs->command("conference ".$confname." play /tmp/announcement.raw");
            }
        }
    }

    # reset the alarm 
    alarm $timeout;
} or die "Error setting SIGALRM handler: $!\n";

sub usage()
{
    print "Usage: $0 [-p pass] [-P port] [-H host] [-d device] [-b baud]\n";
    print "example: $0 -p password -P 8021 -H localhost -d /dev/ttyS0 -b 38400\n";
    exit;
}


sub checkArgs()
{
    getopts("p:P:H:d:b:h",\%options);
    usage() if defined $options{h};
    $password = $options{p} if defined $options{p};
    $host = $options{H} if defined $options{H};
    $port = $options{P} if defined $options{P};
    $device = $options{d} if defined $options{d};
    $baud = $options{b} if defined $options{b};
    
    if(! defined $device || ! defined $password || 
       ! defined $host || ! defined $port) {
        usage();
        exit;
    }
}


checkArgs();
$ptt_port = Device::SerialPins->new($device);
releasePTT();
es_connect();
alarm $timeout;



$SIG{INT} = "byebye";        # traps keyboard interrupt (^C)

sub byebye {
    if(defined $fs) {
        $fs->command("pa hangup");
    }
    exit();
}


if(defined $fs) {
    $fs->command("pa call ".$extension);
} else {
    print "Unable to start portaudio channel\n";
}

$lastcallsign = time;

while (1) {
    if(defined $fs) {
        my $reply = $fs->readhash(undef);
        if ($reply->{socketerror}) {
            es_connect();
        }

        if($reply->{body}) {
            my $myhash = $reply->{event};

            if ($myhash->{'event-name'} eq "HEARTBEAT") {
                $lastheartbeat = time;
            } elsif ($myhash->{'event-subclass'} eq "conference::maintenance") {
                if($myhash->{'conference-name'} eq $confname) {
                    if($myhash->{'caller-channel-name'} =~ m/^portaudio/) {
                        # this is from the radio
                        if($myhash->{'action'} eq 'dtmf') {
                            switch($myhash->{'dtmf-key'}) {
                                # I will be adding some "dial" instructions for autopatch
                                # and maybe some other settings here
                            }
                        }
                    } else {
                        # this is from everyone else
                        if ($myhash->{'action'} eq 'start-talking') {
                            print "The port is talking! keying mic\n";
                            $lasttx = time;
                            pressPTT();
                        } elsif ($myhash->{'action'} eq 'stop-talking') {
                            print "The port stopped talking! releasing mic\n";
                            if(defined $courtesy_tone) {
                                $fs->command("conference ".$confname." play ".$courtesy_tone);
                                $releasePTT++;
                            }
                        }                           
                    }
                    
                    if($myhash->{'action'} eq 'dtmf') {
                        print "conf: $myhash->{'conference-name'}\tmember: $myhash->{'member-id'}\tDTMF: $myhash->{'dtmf-key'}\n";
                    } elsif ($myhash->{'action'} eq 'play-file') {
                        print "conf: $myhash->{'conference-name'}\taction: $myhash->{'action'}\tfile: $myhash->{'file'}\n";
                    } elsif ($myhash->{'action'} eq 'play-file-done') {
                        print "conf: $myhash->{'conference-name'}\taction: $myhash->{'action'}\tfile: $myhash->{'file'}\n";
                        if($releasePTT>0) {
                            $releasePTT--;
                        }
print "release PTT: $releasePTT\n";
                        if($releasePTT==0) {
                            releasePTT();
                        }
                    } else {
                        print "conf: $myhash->{'conference-name'}\tmemid: $myhash->{'member-id'}\taction: $myhash->{'action'}\tCLID: $myhash->{'caller-caller-id-number'}\n";
                    }
                } else {
                    print "conf: $myhash->{'conference-name'}\tmemid: $myhash->{'member-id'}\taction: $myhash->{'action'}\tCLID: $myhash->{'caller-caller-id-number'}\n";
                }
            } else {
                print Dumper $myhash;
            }
        }
    } else {
        es_connect();
    }
}
