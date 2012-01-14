#!/bin/sh
#
# extract-fs-vars-from-source-tree.sh
#
# Walks the FreeSWITCH source tree
#
#
# NOTES
# Be sure to specify your source and temp directories below
# Also, make this script and create-chanvars-html-page.pl executable
#
# MC 2009-04-02

# Change these vars if your directories are different
#SRCDIR=/usr/src/freeswitch/src
SRCDIR=/Users/michaelcollins/workspace/fs/freeswitch/src
TMPDIR=/tmp
HTMLDIR=/usr/local/freeswitch/htdocs

# Grep the variables, then grep the aliases in the header files
echo "Searching source tree for channel variable set/get..."
grep -rn "[sg]et_variable(" $SRCDIR/* | grep -v Binary | grep -v svn > $TMPDIR/get-set-vars.txt
grep -rn "[sg]et_variable_partner(" $SRCDIR/* | grep -v Binary | grep -v svn >> $TMPDIR/get-set-vars.txt

echo "Searching source tree for switch_channel_execute_on statements..."
grep -rn "switch_channel_execute_on(" $SRCDIR/* | grep -v Binary > $TMPDIR/execute-on-vars.txt 

echo "Searching source tree for event_get_header channel variables... "
grep -rn "switch_event_get_header(var_event" $SRCDIR/* | grep -v Binary > $TMPDIR/event-vars.txt

echo "Getting header file definitions..."
grep -n "_VARIABLE" $SRCDIR/include/*h > $TMPDIR/header-defs.txt

# launch perl script here
echo "Creating HTML file in $TMPDIR directory"
./create-chanvars-html-page.pl --srcdir=$SRCDIR --tmpdir=$TMPDIR --htmldir=$HTMLDIR
