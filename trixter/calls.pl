#!/usr/bin/perl
# -*- mode:cperl; tab-width:4; c-basic-offset:4; c-indent-level:4; indent-tabs-mode:nil;  -*-

#
# this script will look for call setup/tear down events and conference join/part events and send them
# to a webserver which can then update a database so you have realtime counts and all of active channels
# across multiple switches.  It also allows you to know who is in without making many XML-RPC calls so you
# can use XML-RPC to hang up the calls or whatever
# 

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
my $timeout   = 30;                                  # seconds to expect a heartbeat event or reconnect
my $url       = "http://localhost/activecalls.php";  # the url to post the data to use http://user:pass@host/script for auth credentials


##
# dont touch  these
##
my $fs;
my $lastheartbeat;
my @uuids;
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
        $fs->sendmsg({'command' => 'event plain heartbeat channel_hangup codec CUSTOM conference::maintenence'}); 
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

        } elsif ($myhash->{'event-name'} eq "CODEC") { ## New call setup
          if(!$uuids->{$myhash->{'unique-id'}}) {
            print $myhash->{'unique-id'}." has called\n";
            $uuids->{$myhash->{'unique-id'}}=1;
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
          elsif($uuids->{$myhash->{'unique-id'}}==2) {
            delete $uuids->{$myhash->{'unique-id'}}; # we get a codec event after hangup
          }
          


        } elsif ($myhash->{'event-name'} eq "CHANNEL_HANGUP") { ## hangup event
          print $myhash->{'unique-id'}." has hung up\n";
          $uuids->{$myhash->{'unique-id'}}=2; # we do this so that the final codec event can delete it
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
          
          
	    } elsif ($myhash->{'event-subclass'} eq "conference::maintenence") {   ## deal with Conference stuff
          if(($myhash->{'action'} eq 'add-member') || ($myhash->{'action'} eq 'del-member')) {  ## join/part
            print $myhash->{'unique-id'} ." ". $myhash->{'action'}." in conference " . $myhash->{'conference-name'}."\n";
            $urlstr="";
            while ( my ($key, $value) = each(%$myhash) ) {
              $urlstr .= "&$key=".uri_escape($value);
            }

            my $req = HTTP::Request->new(POST => $url);
            $req->content_type('application/x-www-form-urlencoded');
            $req->content($urlstr);
            my $res = $ua->request($req);
            if(!$res->is_success) {
              print "Error posting ".$myhash->{'action'}." to webserver got ".$res->status_line."\n";
            }

          }
	    } else {  ## Unknown event
          print "EVENT NAME: " . $myhash->{'event-name'} . "\n";
          print Dumper $myhash;
          #	    print "$reply->{body}\n"; # print out what was sent, myhash is translated by Client.pm
	    }
	    
      } 
    }
}

