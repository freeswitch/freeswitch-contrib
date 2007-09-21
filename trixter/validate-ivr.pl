#!/usr/bin/perl
# -*- mode:perl; tab-width:4; c-basic-offset:4; c-indent-level:4; indent-tabs-mode:nil;  -*-


use XML::LibXML;
use POSIX ':signal_h';


# initialize the parser
# my $parser = new XML::LibXML;


my $file = shift @ARGV;
my $tree = XML::LibXML->new()->parse_file($file);
my $root = $tree->getDocumentElement;
my @menudata = $root->getElementsByTagName('menu');
my $menu_top = shift @ARGV;



sigaction SIGALRM, new POSIX::SigAction sub { 
    die "TIMEOUT";
} or die "Error setting SIGALRM handler: $!\n";



sub printMenu($) {
    my $menu = shift;
    
    my $name = $menu->getAttribute('name');
    my $greet_long = $menu->getAttribute('greet-long');
    my $greet_short = $menu->getAttribute('greet-short');
    my $invalid_sound = $menu->getAttribute('invalid-sound');
    my $exit_sound = $menu->getAttribute('exit-sound');
    
    print "$name\n";
    
    
    my @entries  = $menu->getElementsByTagName('entry');
    foreach my $entry (@entries) {
        my $action = $entry->getAttribute('action');
        my $digits = $entry->getAttribute('digits');
        my $param  = $entry->getAttribute('param');
        
        if($action eq undef) {
            print "\tAction was undefined, skipping\n";
        } else {
            print "\taction: $action";
            print "\tdigits: $digits" unless (! defined $digits);
            print "\tparam: $param" unless (! defined $param);
            print "\n";
        }
    }
    
#    my $common_name = $name_node[0]->getFirstChild->getData;
#    my @c_node  = $menu->getElementsByTagName('conservation');
#    my $status =  $c_node[0]->getAttribute('status');
    
}


sub findMenu($) {
    my $menuName = shift;
    foreach my $menu (@menudata) {
        if ($menuName eq $menu->getAttribute('name')) {
            return $menu;
        }
    }
    print "Unable to locate menu $menuName!\n";
}
    
sub runMenu($) {
    my @menu = shift;

    my $tries = 0;

    my $max_failures  = @menu[0]->getAttribute('max-failures');
    my $timeout       = @menu[0]->getAttribute('timeout');
    my $greet_long    = @menu[0]->getAttribute('greet-long');
    my $greet_short   = @menu[0]->getAttribute('greet-short');
    my $invalid_sound = @menu[0]->getAttribute('invalid-sound');
    my $exit_sound    = @menu[0]->getAttribute('exit-sound');


    print "Playing $greet_long\n";
    while (true) {
        printMenu(@menu[0]);
        alarm $timeout;

        eval {
            print "Enter your choice: ";
            $choice = <STDIN>;
            chomp($choice);
            
            my @entries  = @menu[0]->getElementsByTagName('entry');
            my $action;
            my $param;
            foreach my $entry (@entries) {
                if($entry->getAttribute('digits') eq $choice) {
                    $action = $entry->getAttribute('action');
                    $param = $entry->getAttribute('param');
                    last;
                }
            }
            
            if($action eq "menu-sub") {
                runMenu(findMenu($param));
            } elsif ($action eq "menu-exit") {
                print "Playing $exit_sound\n";
                exit;
            } elsif ($action eq "menu-back") {
                return;
            } elsif ($action eq "menu-top") {
                runMenu(findMenu($menu_top));
            } elsif ($action eq "menu-exec-api") {
                print "EXEC: $param\n";
            } elsif ($action eq "menu-call-transfer") {
                print "TRANSFER: $param\n";
            } elsif ($action eq "menu-play-sound") {
                print "Playing $param\n";
            } else {
                print "Action $action is not mapped!\n" unless (! defined $action);
                print "Playing $invalid_sound\n";
                $tries++;
            }
        };

        if ($@ =~ /TIMEOUT/) {
            print "Timeout\n";
            $tries++;
        }
            
        if($tries lt $max_failures) {
            print "Playing $greet_short\n";
        } else {
            last;
        }

    }
    print "Playing $exit_sound\n";
    exit;

}

runMenu(findMenu($menu_top));
