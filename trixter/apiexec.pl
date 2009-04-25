#!/usr/bin/perl
# -*- mode:cperl; tab-width:4; c-basic-offset:4; c-indent-level:4; indent-tabs-mode:nil;  -*-

#
# this script will execute api commands via xml-rpc.  This can be handy if you have 
# some other program that needs to execute things but you dont have a need or ability
# to write a full program.  This makes it trivial to execute commands from cron/windows scheduler
# or from some other interface, especially if you are not a developer.
#
# the api commands are just as you would type them in the CLI, the output is printed to STDOUT
#
# examples:
#   apiexec.pl -h
#   apiexec.pl status
#   apiexec.pl -p password -P 8021 -H 1.2.3.4 "originate sofia/mydomain.com/123\@10.0.0.1 &playback(/sounds/mysound.raw) XML default \"CID NAME\" \"12345678980\""

use FreeSWITCH::Client;
use Getopt::Std;


# Default values
my $password  = "ClueCon";                           # the password for event socket
my $host      = "localhost";                         # the hostname to connect to
my $port      = 8021;                                # the port to connect to



my %options=();
my $fs;

sub usage()
  {
    print "Usage: $0 [-p pass] [-P port] [-H host] [-h] <command>\n";
    exit;
  }
  
  
  # this connects to the event socket
  sub es_connect()
    {
      eval {
        $fs = init FreeSWITCH::Client {-password => $password, -host => $host, -port => $port};
        if(defined $fs) {
          $fs->sendmsg({'command' => 'event plain'}); 
        }
      } or die "Error connecting: $!\n";
    }
    
    
    getopts("p:P:H:h",\%options);
    
    usage() if(!$ARGV[0] || defined $options{h});
    $password = $options{p} if defined $options{p};
    $host = $options{H} if defined $options{H};
    $port = $options{P} if defined $options{P};
    
    es_connect();
    my $reply = $fs->command($ARGV[0]);
    if ($reply->{socketerror}) {
      die "socket error";
    }
    print "$reply\n";

