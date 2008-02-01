#!/usr/bin/perl
# -*- mode:cperl; tab-width:4; c-basic-offset:4; c-indent-level:4; indent-tabs-mode:nil;  -*-


print "Enter the regexp you wish to test: ";
$regexp = <STDIN>;
chomp($regexp);

print "Enter the value you wish to test: ";
$value = <STDIN>;
chomp($value);

if ($value =~ m/$regexp/) {
  print "$value matches $regexp\n";
  $x=1;
  while (defined $$x) {
    print "capture $x: $$x\n";
    $x++;
  }
} else {
  print "$value does not match $regexp\n";
}

