#!/usr/bin/perl
# -*- mode:cperl; tab-width:4; c-basic-offset:4; c-indent-level:4; indent-tabs-mode:nil;  -*-

# Copyright (C) 2009, Bret McDanel <trixter AT 0xdecafbad.com>

# I wanted an automated method to generate a modules.conf.xml based on the modules.conf
# that you build the code from.  This is ideal for new builds but not very useful
# after that.

# to run:
# ./makemodconf.pl /usr/src/freeswitch/modules.conf > /usr/local/freeswitch/conf/autoload_configs/modules.conf.xml

# this is for modules that are not directly loaded by modules.conf.xml
my @exclude = (
               '^mod_spidermonkey_.*$',
              );

# this is for things that have odd directory locations
my %sectionfix = (
                  '../../libs/openzap' => 'endpoints'
                 );


my %modules = ();

sub trim($)
  {
    my $string = shift;
    $string =~ s/^\s+//;
    $string =~ s/\s+$//;
    return $string;
  }
  
  sub is_excluded($)
    {
      my $modname = shift;
      foreach my $regexp (@exclude) {
        if($modname =~ /$regexp/) {
          return 1;
        }
      }
      return 0;
    }
    
    sub fix_section($)
      {
        my $section = shift;
        if (exists $sectionfix{$section}) {
          return $sectionfix{$section};
        } else {
          return $section;
        }
      }
      
      
      if ($#ARGV != 0) {
        print "Usage: $0 modules.conf\n";
        exit;
      }
      
      my $infile = shift @ARGV;
      
      
      open( INFILE, "< $infile" ) or die "Can't open $infile $!";
      
      while( <INFILE> ) {
        $_ = trim($_);
        
        $_ =~ /^(#?)(.*)\/(.*)$/;
        
        my $comment = $1;
        my $section = fix_section($2);
        my $modname = $3;
        
        
        if(!is_excluded($modname)){
          push (@{$modules{$section}},[$modname,$comment]);
        }
      }

      print "<configuration name=\"modules.conf\" description=\"Modules\">\n";
      print "  <modules>\n";

      foreach my $section (sort(keys %modules)) {
        print "\n\t<!-- $section -->\n";
        foreach (@{$modules{$section}}) {
          my $comment = @$_[1];
          my $modname = @$_[0];
          if($comment eq "#") {
            print "\t<!-- <load module=\"$modname\"/> -->\n";
          } else {
            print "\t<load module=\"$modname\"/>\n";
          }
        }
      }
      print "  </modules>\n";
      print "</configuration>\n";
