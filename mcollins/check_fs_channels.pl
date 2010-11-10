#!/usr/bin/perl
#
# nagios-plugin-template.pl
#
# Simple template for using ESL to create Nagios plugins
#
# Requires the nagios-plugins package from http://nagiosplugins.org/
#  Be sure to run ./configure --enable-perl-modules or you won't get too far!
#

use strict;
use warnings;
use lib '/usr/local/nagios/libexec/' ;
use utils qw($TIMEOUT %ERRORS &print_revision &support);
use lib '/usr/src/freeswitch.git/libs/esl/perl';
use ESL;
use Getopt::Long;

$|++;       # No buffering, please

## Setup command line argument vars
## FreeSWITCH connection
my $host;   # FreeSWITCH IP address (default="localhost")
my $port;   # FreeSWITCH port (default="8021")
my $pass;   # Password to log in to event socket (default="ClueCon")

## Nagios specific options
my $version;             # version of the Nagios plugin
my $help;                # help flag
my $timeout;             # timeout in milliseconds
my $warning_threshold;   # warning threshold, integer
my $critical_threshold;  # critical threshold, integer
my $verbose;             # verbosity level (default=non-verbose)

my $opts_ok = GetOptions (
   'h'      => \$help,      'help'          => \$help,
   "H=s"    => \$host,      "host=s"        => \$host,
   "p=i"    => \$port,      "port=i"        => \$port,
   "P=s"    => \$pass,      "password=s"    => \$pass,
   "t=i"    => \$timeout,   "timeout=i"     => \$timeout,
   'v'      => \$verbose,   'verbose'       => \$verbose,
  
);

## Check command line args and set defaults
if ( ! $opts_ok ) { &usage;                     }
if ( $help      ) { &usage;                     }

if ( ! $host    ) { $host       = 'localhost';  }
if ( ! $port    ) { $port       = '8021';       }
if ( ! $pass    ) { $pass       = 'ClueCon';    }
if ( ! $timeout ) { $timeout    = $TIMEOUT;     }

## Attempt to connect
my $con = new ESL::ESLconnection($host, $port, $pass);
if ( ! $con ) {
    print "Unable to connect to FreeSWITCH on $host:$port\n";
    exit $ERRORS{'WARNING'};
} else {
   # print "Successfully connected to FreeSWITCH on $host:$port\n";
}

## Do a show channels
my $chans;
my $calls;
my $e = $con->api('show','channels count');
if ( $e->getBody() =~ m/(\d+)/ ) {
    $chans = $1;
} else {
    $chans = 0;
}

## Do a show calls
$e = $con->api('show','calls count');
if ( $e->getBody() =~ m/(\d+)/ ) {
    $calls = $1;
} else {
    $calls = 0;
}

print "$chans / $calls  (chans / calls) \n";

## Proper exit
exit $ERRORS{'OK'};

sub usage {
    print $0;
    print <<'EOF';
 [-H|--host host] -P password [-p|--port port]
 [-t timeout] [-v] [-h] [--help]

    Options:

     -h,--help 
        Print detailed help screen
     -V
        Print version information
     -H,--host
        FreeSWITCH server host name or address (default $host)
     -P 
        FreeSWITCH password (default $pass)
     -p
        FreeSWITCH event socket port (default $port)
     -t 
        Number of seconds to wait while trying to connect to FreeSWITCH server 
        (default $timeout)
     -v
        Verbose
EOF
    exit(0);
    
} # sub usage
