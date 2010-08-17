set FREESWITCH_DIR "/home/quentusrex/testbuild/freeswitch/clean-test"
set TEMP_DIR "./test_temp"
set FREESWITCH_LOG "$TEMP_DIR/log/"
set FREESWITCH_CONF "$FREESWITCH_DIR/conf/"
set FREESWITCH_DB "$TEMP_DIR/db/"
set FREESWITCH_RUN "$TEMP_DIR/run/"




send_user "Testing Startup"

spawn $FREESWITCH_DIR/freeswitch -run $FREESWITCH_RUN -log $FREESWITCH_LOG -db $FREESWITCH_DB -conf $FREESWITCH_CONF -nonat

expect "freeswitch@"
expect "> "

send "...\r"

expect "Ending sessions"

wait
send_user "Done"



