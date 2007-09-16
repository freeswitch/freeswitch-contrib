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
my $conntime  = 30;                           # seconds to wait between connection attempts
my $CIDNAME   = "its me";                     # name to use when originating a call
my $CIDNUM    = "1234567890";                 # number to use when originating a call
my $TIMEOUT   = 30;                           # timeout to wait for answer

# array of the regexps for channels that match, useful if you dont want to do this for
# all calls on your switch
my @channels = (
                '^sofia\/mydomain.com\/\d+\@192.168.0.6$',
                '^sofia/external/888@conference.freeswitch.org$',
);

##
# dont touch  these
##
my $fs;
my $lastheartbeat;
my $lastconnect;


sub watchedChannel($) {
  my $channame = shift;

print "Checking $channame\n";


  foreach $regexp (@channels) {
    if($channame =~ /$regexp/) {
      return 1;
    }
  }
  return undef;
}


sub do_ring($) {
  my $myhash = shift;

  print "Got RING for $myhash->{'channel-name'} - $myhash->{'unique-id'}\n";

  # here we are going to just hang it up since its just a silly example
  # but for variety lets wait 2 seconds before it dies
  $fs->sendmsg({'command' => "api sched_hangup +2 $myhash->{'unique-id'}"});
}


sub do_answer($) {
  my $myhash = shift;

  print "Got ANSWER for $myhash->{'channel-name'}\n";

  # here we are going to just hang it up since its just a silly example
  $fs->sendmsg({'command' => "api killchan $myhash->{'unique-id'}"});
}



sub do_hangup($) {
  my $myhash = shift;

  print "Got HANGUP for $myhash->{'channel-name'} $myhash->{'unique-id'}\n";

  # hey it was a hangup, lets call someone now!
  $cmd = "api originate sofia/mydomain.com/123\@192.168.0.6 &playback(/sounds/conf-enter.raw) XML default \"$CIDNAME\" \"$CIDNUM\"";
  $fs->sendmsg({'command' => $cmd});
}



# this connects to the event socket
sub es_connect() {
    if(! defined $lastconnect) {
      $lastconnect = time - $conntime;
    }

    if($lastconnect <= (time - $conntime)) {
      $lastconnect = time;
      print "Connecting to $host:$port\n";
      eval {
        $fs = init FreeSWITCH::Client {-password => $password, -host => $host, -port => $port};
        if(defined $fs) {
          $fs->sendmsg({'command' => 'event plain CHANNEL_STATE CHANNEL_ANSWER heartbeat CHANNEL_HANGUP all'});
          $lastheartbeat = time;
        }
      } or do {
        print "Error connecting - waiting for retry\n";
      }
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
          
	    } elsif (watchedChannel($myhash->{'channel-name'})) {
          if ($myhash->{'event-name'} eq "CHANNEL_ANSWER") {   ## deal with answers
            do_answer($myhash);
          } elsif ($myhash->{'event-name'} eq "CHANNEL_STATE" && $myhash->{'channel-state'} eq "CS_RING") {
            do_ring($myhash);
          } elsif ($myhash->{'event-name'} eq "CHANNEL_HANGUP") { ## last thing done on a channel
            do_hangup($myhash);
          } else {  ## Unknown event
            print Dumper $myhash;
            #	    print "$reply->{body}\n"; # print out what was sent, myhash is translated by Client.pm
          }
	    } # end channel-name filter
      }
    }
}

