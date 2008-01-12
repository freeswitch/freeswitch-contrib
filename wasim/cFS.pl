#!/usr/bin/perl

# Convergence FreeSwitch Tools Version 7.0 : AGI
# (c) MMII Convergence. All rights reserved.
# <info@convergence.pk> http://www.convergence.pk

# This program is free software, distributed under the terms of
# the GNU General Public License.http://www.gnu.org/licenses.html

#$0 -t day -s 30 -e 20 -p 92 will get stats on daily basis for the last 30 to 20 hours for number beginning with 92 
# make sure end_timestamp is indexed in the db

#use warnings;
use strict;
use DBI();
use Getopt::Long;

GetOptions (
    't|time:s' => \(my $t = 'hour'),
    's|start:i' => \(my $i = '24'),
    'e|end:i' => \(my $j = '0'),
    'p|prefix:s' => \(my $prefix = 'WHERE 1')
);

my $prefix = "WHERE destination_number like '$prefix%' " if ($prefix ne 'WHERE 1');

my $dbh		= DBI->connect("DBI:mysql:database=freeswitch;host=x.x.x.x","USER","PASS") or &die_call;

my ($tcalls,$tans,$tmins,$tamins,$tacd,$tasr,$tbpct,$tbmins,$tpdd);
while ($i > $j) {
	&get_stats($i);
	$i--;
}
$tasr 	= eval { ( sprintf("%.2f", $tans / $tcalls * 100)) };
$tacd 	= eval { ( sprintf("%.2f", $tamins / $tans)) };
$tbpct 	= eval { ( sprintf("%.2f", $tamins / $tmins * 100)) };
$tpdd	= eval { ( sprintf("%.2f", (($tbmins - $tamins) * 60) / $tans)) };
	
print "\t\t\t       ------ \t        -----\t     ------\t        ------ \t     ---- \t     -----\t       ------\n" if $tans;
print "\t\t\ttotal: $tcalls\tanswer: $tans\tasr: $tasr%\tminute: $tamins\tacd: $tacd\tpdd: $tpdd\tbill%: $tbpct%\n" if $tans;

exit;

sub get_stats {
	my ($calls,$ans,$mins,$amins,$acd,$asr,$bpct,$pdd,$bmins);
	my $time	= shift;
	my $ntime	= $time-1;
	my $stm		= "select count(*) as calls, sum(duration)/60 as mins from cdr $prefix and end_timestamp between 
				date_sub(now(), interval $time $t) and date_sub(now(), interval $ntime $t)";
	my $res 	= &do_db($stm);
	$calls 		= $res->{'calls'};
	$tcalls		= $tcalls + $calls;
	$mins 		= sprintf("%.0f", $res->{'mins'});
	$tmins		= $tmins + $mins;

	$stm		= "select count(*) as ans, sum(billsec)/60 as amins, sum(duration)/60 as bmins,date_sub(now(), 
				interval $ntime $t) as start from cdr $prefix AND HANGUP_CAUSE='NORMAL_CLEARING' 
				and end_timestamp between date_sub(now(), interval $time $t) and date_sub(now(), interval $ntime $t)";
	$res 		= &do_db($stm);
	my $when	= $res->{'start'};
	$ans		= $res->{'ans'};
	$tans		= $tans + $ans;
	$amins		= sprintf("%.0f", $res->{'amins'});
	$tamins		= $tamins + $amins;
	$bmins		= sprintf("%.0f", $res->{'bmins'});
	$tbmins		= $tbmins + $bmins;

	$asr 	= eval { ( sprintf("%.2f", $ans / $calls * 100)) };
	$acd 	= eval { ( sprintf("%.2f", $amins / $ans)) };
	$bpct 	= eval { ( sprintf("%.2f", $amins / $mins * 100)) };
	$pdd	= eval { ( sprintf("%.2f", (($bmins - $amins) * 60) / $ans)) };
	
	print "$time $when\tcalls: $calls\tanswer: $ans\tasr: $asr%\tminute: $amins\tacd: $acd\tpdd: $pdd\tbill%: $bpct%\n" if $ans;
}

sub do_db {
	my $sql         = shift;
#	print "$sql\n";
	my $get         = $dbh->prepare($sql) or die "Couldn't prepare statement: " . $dbh->errstr;
	$get->execute();
	my $res         = $get->fetchrow_hashref();
	$get->finish();
	return $res;
}
