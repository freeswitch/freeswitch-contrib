#!/usr/bin/perl
# -*- mode:cperl; tab-width:4; c-basic-offset:4; c-indent-level:4; indent-tabs-mode:nil;  -*-
use FreeSWITCH::Client;
use POSIX ':signal_h'; # used for alarm to ensure we get heartbeats
use Data::Dumper; # used to print out myhash debug info


# configure these
my $password  = "ClueCon";                    # the password for event socket
my $host      = "localhost";                  # the hostname to connect to
my $port      = 8021;                         # the port to connect to
my $timeout   = 30;                           # seconds to expect a heartbeat event or reconnect

# hash for music, default must be defined, if you define other entities then the conference name will be used to match against that
my %soundfile = (
                 'default' => '/sounds/fpm-calm-river.wav',
                 '123'     => '/sounds/welcome.raw',
                );



##
# dont touch  these
##
my $fs;
my $lastheartbeat;


# this connects to the event socket
sub es_connect()
  {
    print "Connecting to $host:$port\n";
    eval {
      $fs = init FreeSWITCH::Client {-password => $password, -host => $host, -port => $port};
      if(defined $fs) {
        $fs->sendmsg({'command' => 'event plain heartbeat CUSTOM conference::maintenence'});
        $lastheartbeat = time;
      }
    } or do {
      print "Error connecting - waiting for retry\n";
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
  
  # reset the alarm
  alarm $timeout;
} or die "Error setting SIGALRM handler: $!\n";



es_connect();
alarm $timeout;

while (1) {
    if(defined $fs) {
      my $reply = $fs->readhash(undef);
      if ($reply->{socketerror}) {
	    es_connect();
      }
      
      if ($reply->{body}) {
	    $myhash = $reply->{event};
        
	    if ($myhash->{'event-name'} eq "HEARTBEAT") {   ## Deal with heartbeats
          $lastheartbeat = time;
          print "Got a heartbeat\n";
          
	    } elsif ($myhash->{'event-subclass'} eq "conference::maintenence") {   ## deal with Conference stuff
          
          if($myhash->{'action'} eq 'dtmf') {  ## DTMF event
		    print "conf: $myhash->{'conference-name'}\tmember: $myhash->{'member-id'}\tDTMF: $myhash->{'dtmf-key'}\n";
		    if(defined $soundfile{$myhash->{'conference-name'}}) {
              $fs->sendmsg({'command' => "api conference $myhash->{'conference-name'} play $soundfile{$myhash->{'conference-name'}}"});
		    } else {
              $fs->sendmsg({'command' => "api conference $myhash->{'conference-name'} play $soundfile{'default'}"});
		    }
		    
          } else {  ## Just print out all other events
		    print "conf: $myhash->{'conference-name'}\tmemid: $myhash->{'member-id'}\taction: $myhash->{'action'}\tCLID: $myhash->{'caller-caller-id-number'}\n";
          }
	    } else {  ## Unknown event
          print Dumper $myhash;
          #	    print "$reply->{body}\n"; # print out what was sent, myhash is translated by Client.pm
	    }
	    
      } 
    }
}

