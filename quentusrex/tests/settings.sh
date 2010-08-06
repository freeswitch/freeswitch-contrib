#!/bin/bash

FREESWITCH_DIR="/home/quentusrex/testbuild/freeswitch/clean-test"
ACCEPTED_TESTS=`find ./accepted/ -type f -name "*.sh" -not -name "*~"`
CONTRIB_TESTS=`find ./contrib/ -type f -name "*.sh" -not -name "*~"`
TEMP_DIR="./test_temp"
FREESWITCH_LOG="$TEMP_DIR/log/"
FREESWITCH_CONF="$FREESWITCH_DIR/conf/"
FREESWITCH_DB="$TEMP_DIR/db/"
FREESWITCH_RUN="$TEMP_DIR/run/"
FREESWITCH_WAIT=20


