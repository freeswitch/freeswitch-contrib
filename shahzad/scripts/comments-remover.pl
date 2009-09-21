#!/usr/bin/perl -w
# This scripts removes comments from .c .cpp and .h files
#
# Usage: comments-remover.pl <original-filename> <resultant-filename>
#
# For example,
# comments-remover.pl abc.c /tmp/abc.c
#
# Author:  Muhammad Shahzad <shaheryarkh@googlemail.com>
# License: MPL 1.1
# Updated: Sept. 20, 2009.
#
use strict;
use warnings;

my $usage = "Usage: $0 <original-filename> <resultant-filename>\n";

die $usage if @ARGV != 2;

open(SRC, $ARGV[0]);
open(DST, ">".$ARGV[1]);

$/ = undef;
$_ = <SRC>;

s#/\*.*?\*/|//[^\n]*|("(\\.|[^"\\])*"|'(\\.|[^'\\])*'|.[^/"'\\]*)#defined $1 ? $1 : ""#gse;

print DST;

close(SRC);
close(DST);

