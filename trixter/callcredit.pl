#!/usr/bin/perl
# -*- mode:cperl; tab-width:4; c-basic-offset:4; c-indent-level:4; indent-tabs-mode:nil;  -*-

#
# this script will look for call setup/tear down events and every INTERVAL query the server for credit 
# on the calls in progress, if it gets a message saying to hangup a call it will - implying credit no
# longer exists
#
# this script also informs the webserver of the call setup/tear down info so that it can keep track and 
# later only post a list of UUIDs for the webserver to process.  
# 
# the webserver should return the following
# HANGUP: 1234-5678-90ab-cdef
# IGNORE: fedc-ba09-8765-4321
#
# to hangup the first or stop getting reports about the 2nd uuid
#
# uuids are passed to the webserver in a php style array named uuid so its easy to loop through them and 
# check them for validity

use FreeSWITCH::Client;
use POSIX ':signal_h'; # used for alarm to ensure we get heartbeats
use Data::Dumper; # used to print out myhash debug info

# use DBI instead of these two for direct database inserts/deletes 
# which is likely to be more efficient, this is however just an example
use LWP::UserAgent; # for posting to the webserver
use URI::Escape ('uri_escape');  # to html encode values


# configure these
my $password  = "ClueCon";                           # the password for event socket
my $host      = "localhost";                         # the hostname to connect to
my $port      = 8021;                                # the port to connect to
my $timeout   = 1;                                   # when setting this make sure its low enough to detect
                                                     # lost heartbeats and high enough that it doesnt hose your 
                                                     # system, also queryInterval is checked each time this runs
                                                     # so you should make queryInterval an interger multiple
my $queryInterval = 3;
my $heartbeatInterval = 30;                          # seconds to expect a heartbeat event or reconnect
my $url       = "http://localhost/activecalls.php";  # the url to post the data to use http://user:pass@host/script for auth credentials


##
# dont touch  these
##
my $fs;
my $lastheartbeat;
my $lastQuery;
my %uuids;
my $ua = LWP::UserAgent->new;
$ua->agent("FreeSWITCH/1.0");


# this connects to the event socket
sub es_connect()
  {
    print "Connecting to $host:$port\n";
    eval {
      $fs = init FreeSWITCH::Client {-password => $password, -host => $host, -port => $port};
      if(defined $fs) {
        # channel_create doesnt have the destination number so we wait for the codec event
        $fs->sendmsg({'command' => 'event plain heartbeat channel_hangup codec'}); 
        $lastheartbeat = time;
      }
    } or do {
      print "Error connecting - waiting for retry\n";
    }

  }


sigaction SIGALRM, new POSIX::SigAction sub { 
  if ($lastheartbeat < (time - $heartbeatInterval)) {
	print "Did not receive a heartbeat in the specified timeout\n";
	if (defined $fs) {
      $fs->disconnect();
      undef $fs;
	}
	es_connect();
  }


  # check the calls
  $uuidstr = "";
  $queryTime = time;
  for my $uuid ( keys %uuids ) {
    if($uuids{$uuid} > 2 && $uuids{$uuid}<= ($queryTime - $queryInterval)) {
      $uuidstr .= "&uuid[]=$uuid";
      $uuids{$uuid}=$queryTime;
    }
  }

  if($uuidstr ne "") {
    my $req = HTTP::Request->new(POST => $url);
    $req->content_type('application/x-www-form-urlencoded');
    $req->content($uuidstr);
    my $res = $ua->request($req);
    if(!$res->is_success) {
      print "Error checking credit got ".$res->status_line."\n";
    }
    @lines = split(/\n/,$res->content);

    foreach $line (@lines) {
      if($line =~ /^HANGUP: (.*)/) {
        print "hanging up $1\n";
        $fs->sendmsg({'command' => "api killchan $1"});
      } elsif ($line =~ /^IGNORE: (.*)/) {
        delete $uuids{$1};
      }

      print "LINE: $line\n\n";
    }

    # XXX we need to figure out how to hangup calls here
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

        } elsif ($myhash->{'event-name'} eq "CODEC") { ## New call setup
          if(!$uuids{$myhash->{'unique-id'}}) {
            print $myhash->{'unique-id'}." has called\n";
            $uuids{$myhash->{'unique-id'}}=time;
            $urlstr="";
            while ( my ($key, $value) = each(%$myhash) ) {
              $urlstr .= "&$key=".uri_escape($value);
            }

            my $req = HTTP::Request->new(POST => $url);
            $req->content_type('application/x-www-form-urlencoded');
            $req->content($urlstr);
            my $res = $ua->request($req);
            if(!$res->is_success) {
              print "Error posting NEWCALL to webserver got ".$res->status_line."\n";
            }
          }
          


        } elsif ($myhash->{'event-name'} eq "CHANNEL_HANGUP") { ## hangup event
          print $myhash->{'unique-id'}." has hung up\n";
          delete $uuids{$myhash->{'unique-id'}}; # we get a codec event after hangup

          $urlstr="";
          while ( my ($key, $value) = each(%$myhash) ) {
            $urlstr .= "&$key=".uri_escape($value);
          }

          my $req = HTTP::Request->new(POST => $url);
          $req->content_type('application/x-www-form-urlencoded');
          $req->content($urlstr);
          my $res = $ua->request($req);
          if(!$res->is_success) {
            print "Error posting HANGUP to webserver got ".$res->status_line."\n";
          }
          
          
	    } else {  ## Unknown event
          print "EVENT NAME: " . $myhash->{'event-name'} . "\n";
          print Dumper $myhash;
          #	    print "$reply->{body}\n"; # print out what was sent, myhash is translated by Client.pm
	    }
	    
      } 
    }
}

