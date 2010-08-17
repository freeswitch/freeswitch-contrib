#!/bin/bash

. ./settings.sh

expect ./contrib/startup.tcl
exit $?
