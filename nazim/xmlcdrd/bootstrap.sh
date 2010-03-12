#!/bin/sh

aclocal
autoconf
autoheader
automake
rm -rf autom4te*.cache

