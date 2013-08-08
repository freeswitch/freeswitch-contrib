#!/usr/bin/perl
use strict;
use warnings;
#
# 2013-08-07 18:05:04.927252 [NOTICE] mod_logfile.c:213 New log started.
#
my $logfile = $ARGV[0];

open(FILEIN,'<',$logfile) or die "$! - $logfile\n";
while(<FILEIN>) {
    chomp;
    if ( m/(.*?\] [^:]+:\d+\s)(.*)/ ) {
    #print "Line: $_\n";
    #print "Part 1: $1\n";
    #print "Part 2: $2\n\n";
    print "$2 $1\n";
    } else {
        print "$_\n";
    }
}

close(FILEIN)
