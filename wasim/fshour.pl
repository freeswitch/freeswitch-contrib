#!/usr/bin/perl

# Convergence FreeSwitch Tools Version 7.0 : AGI
# (c) MMII Convergence. All rights reserved.
# <info@convergence.pk> http://www.convergence.pk

# This program is free software, distributed under the terms of
# the GNU General Public License.http://www.gnu.org/licenses.html

# usage: $0 interval start end prefix
# for example: $0 day 30 20 92 will get stats on daily basis for the last 30 to 30 hours for number beginning with 92
# make sure end_timestamp is indexed in the db

#use warnings;
my $debug=0;
use strict;
use DBI();
use Getopt::Long;

GetOptions (
    't|time:s' => \(my $t = 'hour'),
    's|start:i' => \(my $i = '24'),
    'e|end:i' => \(my $j = '0'),
    'p|prefix:s' => \(my $p = 'WHERE 1'),
    'n|notprefix:s' => \(my $n = 'AND 1'),
    'd|date:s'	=> \(my $d = 'NOW()')
);

$p		= "WHERE destination_number like '$p%' " if ($p ne 'WHERE 1');
$n		= "AND destination_number not like '$n%' " if ($n ne 'AND 1');
my $dbh		= DBI->connect("DBI:mysql:database=freeswitch;host=x.x.x.x","USER","PASS") or &die_call;

my ($tcalls,$tans,$tmins,$tamins,$tacd,$tasr,$tbpct,$tbmins,$tpdd);

while ($i > $j) {
	&get_stats($d,$i);
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
	my $x		= shift;
	my $i		= shift;
	my $j		= $i-1;
	my ($calls,$ans,$mins,$amins,$acd,$asr,$bpct,$pdd,$bmins);
	my $stm		= "SELECT COUNT(*) AS calls, SUM(duration)/60 AS mins FROM cdr $p $n AND end_timestamp BETWEEN DATE_SUB($d, INTERVAL $i $t) AND DATE_SUB($d, INTERVAL $j $t)";
	my $res 	= &do_db($stm);
	$calls 		= $res->{'calls'};
	$tcalls		= $tcalls + $calls;
	$mins 		= sprintf("%.0f", $res->{'mins'});
	$tmins		= $tmins + $mins;

	$stm		= "SELECT COUNT(*) AS ans, SUM(billsec)/60 AS amins, SUM(duration)/60 AS bmins, DATE_SUB($d, INTERVAL $i $t) AS start FROM cdr $p $n AND HANGUP_CAUSE='NORMAL_CLEARING' AND end_timestamp BETWEEN DATE_SUB($d, INTERVAL $i $t) AND DATE_SUB($d, INTERVAL $j $t)";
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
	
	print "$i $when\tcalls: $calls\tanswer: $ans\tasr: $asr%\tminute: $amins\tacd: $acd\tpdd: $pdd\tbill%: $bpct%\n" if $ans;
}

sub do_db {
	my $sql         = shift;
	print "$sql\n" if $debug;
	my $get         = $dbh->prepare($sql) or die "Couldn't prepare statement: " . $dbh->errstr;
	$get->execute();
	my $res         = $get->fetchrow_hashref();
	$get->finish();
	return $res;
}
