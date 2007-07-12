#!/usr/bin/perl
# -*- mode:cperl; tab-width:4; c-basic-offset:4; c-indent-level:4; indent-tabs-mode:nil;  -*-

# 
# this script will look for dtmf events in event socket in async mode (where FS calls this 
# script instead of this script connecting to FS) and fill a buffer, when a timeout occurs it will
# return that entire DTMF sequence, lather, rinse, repeat
#

use FreeSWITCH::Client;
use threads;
use threads::shared;
use POSIX ':signal_h'; # used for alarm to ensure we get heartbeats
use Data::Dumper;

# configure these
my $ip              = "0.0.0.0";             # the IP we listen on 0.0.0.0 is all
my $port            = 8084;                  # the port we listen on
my $firstDigitDelay = 30;                    # seconds to wait from the first digit pressed - 0 disables
my $lastDigitDelay  = 3;                     # seconds to wait from the last digit pressed


##
# dont touch  these
##
my $fs;
my $pid;
my $dtmf : shared ="";
my $firstDigitTS : shared=0;
my $lastDigitTS : shared;
my $done : shared =0;


sub timeoutHandler {
  while(!$done) {
    if($dtmf != "") {
      lock($dtmf);
      $ts = time;
      if (($firstDigitTS > 0) && ($firstDigitTS <= ($ts - $firstDigitDelay))) {
        print "First digit timeout - $dtmf\n";
        $dtmf="";
      } elsif ($lastDigitTS <= ($ts - $lastDigitDelay)) {
        print "Last digit timeout - $dtmf\n";
        $dtmf="";
      }
    }
    sleep 1;
  }

  if($dtmf != "") {
    print "Final DTMF when call died - $dtmf\n";
  }
}


$fs = init FreeSWITCH::Client {} or die "Error $@";

for (;;) {
  $fs->accept($ip,$port);

  if (!($pid = fork)) {
    last;
  }
}

$o = $fs->call_command("answer");
$o = $fs->raw_command("myevents");

$timer = threads->new(\&timeoutHandler);
$timer->detach;


# event-date-timestamp
# event-name DTMF
while(my $r = $fs->readhash(undef)) {
  if ($r->{socketerror}) {
    last;
  }
  
  if ($r->{has_event}) {
    if($r->{'event-name'} == "DTMF") {
      $ts=time;
      # you can use the event TS as opposed to the perl one to eliminate network delay, event queue delay
      # or whatever, I use the perl one to keep the example simple
      
      {
        lock($dtmf);
        if($dtmf == "") {
          $firstDigitTS=$ts;
        }
        $lastDigitTS=$ts;
        $dtmf.=$r->{'event'}->{'dtmf-string'};
      }
      
      print "Got dtmf $r->{'event'}->{'dtmf-string'} at $ts\n";
    } else {
      print Dumper $r->{event};
    }
  }
}

$done=1;
$fs->disconnect();
sleep 3; # wait for the other thread to exit
print "done\n";
