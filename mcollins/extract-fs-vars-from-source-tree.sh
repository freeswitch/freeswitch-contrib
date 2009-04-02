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

# Change these vars if your source directory is different
SRCDIR=/usr/usr/freeswitch.trunk
TMPDIR=/tmp

# Grep the variables, then grep the aliases in the header files
grep -rn "[sg]et_variable(" $SRCDIR/* | grep -v Binary | grep -v svn > $TMPDIR/get-set-vars.txt
grep -n "_VARIABLE" $SRCDIR/src/include/*h > $TMPDIR/header-defs.txt

# launch perl script here
./extract-fs-chanvars.pl --srcdir=$SRCDIR --tmpdir=$TMPDIR