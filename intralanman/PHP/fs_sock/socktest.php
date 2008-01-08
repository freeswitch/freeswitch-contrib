<?php
require_once('fs_sock.php');

// uncommenting this will produce LARGE amounts of output
// define('FS_SOCK_DEBUG', true);

// different connection settings, all of these should work with a fresh install
$array[] = array('host'=>'127.0.0.1');
$array[] = array('host'=>'127.0.0.1', 'port'=>'8021');
$array[] = array('pass'=>'ClueCon', 'timeout'=>7);
$array[] = array(
'host'=>'localhost',
'port'=>'8021',
'pass'=>'ClueCon',
'timeout'=>3,
'stream_timeout'=>0.5
);

// commands to run, initially this was how i caught a few of my bugs
$cmds[] = 'status';
$cmds[] = 'originate';
$cmds[] = 'reloadxml';
$cmds[] = 'invalidapp';
$cmds[] = 'craptest';

$obj = new fs_sock($array[3]);
if ($obj -> auth != true) {
    die(printf("Unable to authenticate\r\n"));
}

if (!$obj -> subscribe_events('ALL')) {
    die(printf("Unable to subscribe to events"));
}

for ($x=0; $x<count($cmds); $x++) {
    $debug_string = sprintf("Return ouput from (%s) is:", $cmds[$x]);
    $return = $obj -> api_command($cmds[$x]);
    $bg_return = $obj -> bgapi_command($cmds[$x]);

    $obj -> debug("$debug_string\r\n");
    $obj -> debug($return);
    $obj -> debug($bg_return);
}



//get $items number of items from the event buffer
$output = array();
$items = 25;
for ($i=0; count($output)<$items; $i++) {
    $count = count($output);
    printf("%s - %s\r\n", $i+1, $count);
    //$output[] = $obj -> read_event();
    $output[] = $obj -> wait_for_event();
}
$obj -> debug($output);




$obj -> sock_close();
?>