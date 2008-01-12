#!/usr/bin/perl -w

# Convergence FreeSwitch Tools Version 7.0 : AGI
# (c) MMII Convergence. All rights reserved.
# <info@convergence.pk> http://www.convergence.pk

# This program is free software, distributed under the terms of
# the GNU General Public License.http://www.gnu.org/licenses.html

use strict;
use DBI();

my @cc 	= ("killall", "-HUP", "freeswitch");
system(@cc) == 0
	or die "$0: system @cc failed: $?";

my $dbh	= DBI->connect("DBI:mysql:database=DB;host=HOST","USER","PASSWORD") 
	or die "$0: Couldn't connect to database: " . DBI->errstr;

my @LS	= `ls -1t /usr/local/freeswitch/log/cdr-csv/Master.csv.*`;
foreach my $line (@LS) {
	chop($line);
	my $stm	= "load data local infile '$line' into table TABLE fields enclosed by '\"' terminated by ','";
	my $ul	= $dbh->prepare($stm)
		or die "$0: Couldn't prepare statement $stm: " . $dbh->errstr;;
	$ul->execute();
	$ul->finish;
	system("cat $line >> /usr/local/freeswitch/log/cdr-csv/FULL_Master.csv");
	unlink $line;
}

exit;
