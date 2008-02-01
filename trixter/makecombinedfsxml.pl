#!/usr/bin/perl
# -*- mode:cperl; tab-width:4; c-basic-offset:4; c-indent-level:4; indent-tabs-mode:nil;  -*-

open (FSXML, "freeswitch.xml") or die "Unable to locate freeswitch.xml - run this in the directory with the conf files";

while ($record = <FSXML>) {
  if ($record =~ /<!--#include "(.*?)"-->/) {
      open(CONF, "$1");
      while (<CONF>) {
        print $_;
      }
      close(CONF);
  } else {
    print $record;
  }
}

close(FSXML);
