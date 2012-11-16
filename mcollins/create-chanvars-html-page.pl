#!/usr/bin/perl
#
# create-chanvars-html-page.pl
#
# Part of the janitorial process. :)
#
# NOTE: This script is used in conjunction with the extract-vars-from-source-tree.sh script
#       Do not run this script manually; use the shell script
#       Specify the source and temp directories in that script
#
# Goes through a bunch of source files to snag all lines with 
# switch_channel_(s|g)et_variable in *.c files
# _VARIABLE is *.h files
#
# Creates a table of vars aliased in *.h (hash, actually)
# Then cycles through each line and creates a data struct:
# key = chan var, val = hashes of source file names
# sub hashes are:
#   key = src file name, val = Array of Arrays
#   Each sub array has two elements: 
#     0 = line number
#     1 = 'get' or 'set'
#
# example:
#                            
# remote_media_ip => { 
#                      "mod_esf.c"    => [[114, "set"]],
#                      "mod_sofia.c"  => [[1050, "get"], [1078, "get"]],
#                      "sofia_glue.c" => [[2812, "set"]],
# }
#
# Also creates an index based on filename like so in a hash:
# key = source file name, val = hash of chanvar names
# sub hashes are:
#   key = chan var name, val = Array of arrays
#   Each sub array has two elements:
#     0 = line number
#     1 = 'get' or 'set'
#
# example:
#
# "mod_nibblebill.c" => {
#                         nibble_account      => [[334, "get"], [632, "get"]],
#                         nibble_rate         => [[333, "get"], [540, "get"]],
#                         nibble_total_billed => [[411, "set"]],
# }
#
# Finally we create a simple HTML file that lets the user easily click around to Fisheye source
#

use strict;
use warnings;
use Data::Dump qw(dump);
use File::Basename;
use Getopt::Long;
use HTML::Tiny;

$|++;

# Get the cmd line options
my $srcdir = '/usr/src/freeswitch.trunk';
my $tmpdir = '/tmp';
my $htmldir = '/usr/local/freeswitch/htdocs';

## Use these as defaults unless command line args are supplied
GetOptions( 'srcdir=s' => \$srcdir,
            'tmpdir=s' => \$tmpdir,
            'htmldir=s' => \$htmldir,
);

my $srcdirlen = length $srcdir;  # calculate this once since it doesn't change

my $headerfile = $tmpdir . "/header-defs.txt";
my $datafile   = $tmpdir . "/get-set-vars.txt";
my $htmlfile   = $htmldir . "/chanvars.html";

#my $site = 'http://fisheye.freeswitch.org/browse/FreeSWITCH.git';
my $site = 'http://fisheye.freeswitch.org/browse/freeswitch.git/src';

my $obvious_exceptions; # regex match for channel variable names we don't care about
$obvious_exceptions = '^(|argv|v?var|var_?name|v?buf|(const )?char.*|inner_var_array.*|arg.*|string|tmp_name|[^_]*_var|\(char \*\) vvar)$';

my %switch_types;	# key = switch type def, val = chan var name
my %channel_vars; 	# key = chan var name, val = HoA
					# HoA: key = filename, val = [line number, set|get]
my %source_idx;         # key = src file name, val = HoA
                                        # HoA: key = var name, val = [line number, set|get]
my %source_files; 	# key = source filename, val = fisheye url where link is found

open(FILEIN,"<",$headerfile) or die "$headerfile - $!\n";
while(<FILEIN>) {
    chomp;
    my @RECIN = split /#define /,$_;
    # debug
    #print "$RECIN[1] \n";
    no warnings;
    $RECIN[1] =~ s/"//g;  # strip quote chars
    my ($key, $val) = split /\s/,$RECIN[1];
    $switch_types{$key} = $val;
    use warnings;
}
close(FILEIN);

# debug
#print dump(\%switch_types);

open(FILEIN,"<",$datafile) or die "$datafile - $!\n";
while(<FILEIN>) {
    chomp;
    next if m/switch_channel_get_variables/;  # get variables != what we want
    next unless ( m/switch_channel_(s|g)et_variable\s?\(/ or m/switch_channel_(s|g)et_variable_partner\s?\(/ );

    next if m/#define|SWITCH_DECLARE/;
    next unless m/\.c(pp)?:/;    # only c and cpp files for now

    ## Extract source file name
    my @RECIN = split /(\.c(pp)?)/,$_;  # split on file name.c or name.cpp
    my ($filename,$dir,$ext) = fileparse($RECIN[0]);
    $filename .= $RECIN[1];    # append .c or .cpp
    # debug 
    #print "$filename\n";
    # trim off the srcdir from beginning of string
    my $reldir = substr $dir, $srcdirlen;
    #print "$filename $dir $ext ($reldir)\n";
    if ( ! exists( $source_files{$filename} ) ) {
        $source_files{ $filename } = $site . $reldir . $filename . '?r=HEAD';
    }
    
    ## Extract line number for this occurrence in this source file
    my $linenum = 0;
    if ( $RECIN[3] =~ m/:(\d+):/ ) {
        $linenum = $1;
    }
    
    ## Determine get or set
    my $setget = 'set';
    if ( $RECIN[3] =~ m/get_variable/ ) {
        $setget = 'get';
    }
    
    ## Extract variable name
    # Set is easier than get because splitting on comma does all the work
    my @temp;
    my $var_idx;
    if ( $setget eq 'set' ) {
	$var_idx = 1;
        @temp = split /,\s*?/,$RECIN[3]; # / second elem is channel variable name
        $temp[1] =~ s/"//g;
    } else {
        # get is trickier - need to stop at first close paren
	$var_idx = $#temp;  # last array element contains the chan var name
        @temp = split /,\s+?/,$RECIN[3]; # / second elem is channel variable name
        $temp[$var_idx] =~ s/"//g;
        $temp[$var_idx] =~ m/^([A-Za-z0-9_]+)/;
        $temp[$var_idx] = $1;
        #debug
        #print "$temp[1]\n";
    }
    my $channel_variable_name = $temp[$var_idx];
    $channel_variable_name =~ s/"//g;   # strip quote chars
    $channel_variable_name =~ s/^\s*//; # strip leading whitespace
    if ( exists( $switch_types{$channel_variable_name} ) ) {
        ## This isn't actually a channel variable name but rather a def from switch_types.h
        $channel_variable_name = $switch_types{$channel_variable_name};
    }
    
    ## Skip obvious exceptions...
    next if $channel_variable_name =~ m/$obvious_exceptions/;
    
    ## populate the hash for this variable, filename and line num & set/get
    push @{ $channel_vars{$channel_variable_name}{$filename} }, [$linenum, $setget]; 
    push @{ $source_idx{$filename}{$channel_variable_name} }, [$linenum, $setget];
}

close(FILEIN);

## Handle the event header vars...
$datafile   = $tmpdir . "/event-vars.txt";
open(FILEIN,'<',$datafile) or die "$datafile - $!\n";
while(<FILEIN>){
    chomp;
    next unless m/\.c(pp)?:/;    # only c and cpp files for now
    my @RECIN = split /(\.c(pp)?)/,$_;  # split on file name.c or name.cpp
    my ($filename,$dir,$ext) = fileparse($RECIN[0]);
    $filename .= $RECIN[1];    # append .c or .cpp

    # debug 
    #print "$filename\n";
    # trim off the srcdir from beginning of string
    my $reldir = substr $dir, $srcdirlen;
    #print "$filename $dir $ext ($reldir)\n";
    if ( ! exists( $source_files{$filename} ) ) {
        $source_files{ $filename } = $site . $reldir . $filename . '?r=HEAD';
    }
    
    ## Extract line number for this occurrence in this source file
    my $linenum = 0;
    if ( $RECIN[3] =~ m/:(\d+):/ ) {
        $linenum = $1;
    }

    ## Extract variable name
    ## if ((var = switch_event_get_header(var_event, "freetdm_outbound_ton")) || (var = switch_core_get_variable("freetdm_outbound_ton")))
    ## Need a little loop, in case there are more than one per line...
    while ( m/switch_event_get_header\(var_event,\s+"(.*?)"/g ) {
	## $1 *should* have the name of the variable...
	#print "Var name is $1...\n";
	my $channel_variable_name = $1;

	## Skip obvious exceptions...
	next if $channel_variable_name =~ m/$obvious_exceptions/;
	
	## populate the hash for this variable, filename and line num & 'event_var'
	push @{ $channel_vars{$channel_variable_name}{$filename} }, [$linenum, 'event_var']; 
	push @{ $source_idx{$filename}{$channel_variable_name} }, [$linenum, 'event_var'];
	
    }
}
close(FILEIN);

## Handle the switch_channel_execute_on vars...
$datafile   = $tmpdir . "/execute-on-vars.txt";
open(FILEIN,'<',$datafile) or die "$datafile - $!\n";
while(<FILEIN>){
    chomp;
    next unless m/\.c(pp)?:/;    # only c and cpp files for now
    my @RECIN = split /(\.c(pp)?)/,$_;  # split on file name.c or name.cpp
    my ($filename,$dir,$ext) = fileparse($RECIN[0]);
    $filename .= $RECIN[1];    # append .c or .cpp

    # debug 
    #print "$filename\n";
    # trim off the srcdir from beginning of string
    my $reldir = substr $dir, $srcdirlen;
    #print "$filename $dir $ext ($reldir)\n";
    if ( ! exists( $source_files{$filename} ) ) {
        $source_files{ $filename } = $site . $reldir . $filename . '?r=HEAD';
    }
    
    ## Extract line number for this occurrence in this source file
    my $linenum = 0;
    if ( $RECIN[3] =~ m/:(\d+):/ ) {
        $linenum = $1;
    }

    my @temp = split /,\s*/,$_;
    my $channel_variable_name = $temp[1];
    
    ## Clean off quotes, parens, semicolons...
    $channel_variable_name =~ s/"|;|\)//g;

    if ( exists( $switch_types{$channel_variable_name} ) ) {
        ## This isn't actually a channel variable name but rather a def from switch_types.h
        $channel_variable_name = $switch_types{$channel_variable_name};
    }

    ## Skip obvious exceptions...
    next if $channel_variable_name =~ m/$obvious_exceptions/;

    ## populate the hash for this variable, filename and line num & set/get
    push @{ $channel_vars{$channel_variable_name}{$filename} }, [$linenum, 'execute_on']; 
    push @{ $source_idx{$filename}{$channel_variable_name} }, [$linenum, 'execute_on'];

    # Debug
    #print "var: $channel_variable_name\n";
}
close(FILEIN);

## Handle the switch_channel_api_on vars...
$datafile   = $tmpdir . "/api-on-vars.txt";
open(FILEIN,'<',$datafile) or die "$datafile - $!\n";
while(<FILEIN>){
    chomp;
    next unless m/\.c(pp)?:/;    # only c and cpp files for now
    my @RECIN = split /(\.c(pp)?)/,$_;  # split on file name.c or name.cpp
    my ($filename,$dir,$ext) = fileparse($RECIN[0]);
    $filename .= $RECIN[1];    # append .c or .cpp

    # debug
    #print "$filename\n";
    # trim off the srcdir from beginning of string
    my $reldir = substr $dir, $srcdirlen;
    #print "$filename $dir $ext ($reldir)\n";
    if ( ! exists( $source_files{$filename} ) ) {
        $source_files{ $filename } = $site . $reldir . $filename . '?r=HEAD';
    }

    ## Extract line number for this occurrence in this source file
    my $linenum = 0;
    if ( $RECIN[3] =~ m/:(\d+):/ ) {
        $linenum = $1;
    }

    my @temp = split /,\s*/,$_;
    my $channel_variable_name = $temp[1];

    ## Clean off quotes, parens, semicolons...
    $channel_variable_name =~ s/"|;|\)//g;

    if ( exists( $switch_types{$channel_variable_name} ) ) {
        ## This isn't actually a channel variable name but rather a def from switch_types.h
        $channel_variable_name = $switch_types{$channel_variable_name};
    }

    ## Skip obvious exceptions...
    next if $channel_variable_name =~ m/$obvious_exceptions/;

    ## populate the hash for this variable, filename and line num & set/get
    push @{ $channel_vars{$channel_variable_name}{$filename} }, [$linenum, 'api_on'];
    push @{ $source_idx{$filename}{$channel_variable_name} }, [$linenum, 'api_on'];

    # Debug
    #print "var: $channel_variable_name\n";
}
close(FILEIN);


#debug
#print dump(\%channel_vars);
#foreach (sort keys %channel_vars) { print "'$_'\n"; }
#print dump(\%source_idx) . "\n";
#print dump(\%source_files) . "\n";

## Test the creation of simple html file
open(HTML,'>',$htmlfile) or die "$htmlfile - $!\n";
my $h = HTML::Tiny->new;
my @links;    # contains the links for the HTML::Tiny p and a tags

## Header Row
#push @links, $h->tr( [ $h->th( 'Channel Variable', 'Source File', 'Links') ] );

## Data Rows
## NOTE: This does a case-insensitive sort
foreach my $chanvar ( sort {lc $a cmp lc $b} keys %channel_vars ) {
    my %this_var = %{ $channel_vars{$chanvar} };
    
    ## Cycle through each source file for this chan var
    foreach ( sort keys %this_var ) {
        ## These keys are source file names
        my @row = ( 
            $chanvar, 
            $h->a( { 
                href => $source_files{$_}, target => '_blank',
		   }, 
                $_ 
            )
        );
        my $sourcename = $_;
        foreach ( @{ $this_var{$_} } ) {
            #$row[2] .= "@{ $_ }" . "<br/>";
            my $linkname = "@{ $_ }";
            my $linenum = @{ $_ }[0];
            my $url = $source_files{$sourcename} . '#to' . $linenum;
            $row[2] .= $h->a( { href => $url, target => '_blank' },
                $linkname
            ) . '<br/>';
        } 
        push @links, $h->tr( [ $h->td(@row) ] );
    }

}

## debug
#print dump(\@links) . "\n";

## Create a table
my $table = $h->table( 
    { 
        border => 1, cellspacing => 5, cellpadding => 1,
    },
    [
        $h->tr( [ $h->th( 'Channel Variable', 'Source File', 'Links') ] ),
        @links,
    ]
);

## Create the HTML
print HTML $h->html(
    [
      $h->head( $h->title( 'FreeSWITCH Source Files' ) ),
      $h->body(
        [
	 $h->h1( { class => 'main' }, 'FreeSWITCH Channel Variables' ),
         $table,
        ]
      )
    ]
);

close(HTML);

print "Operation complete\n";
