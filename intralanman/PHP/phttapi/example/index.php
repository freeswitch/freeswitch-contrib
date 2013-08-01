<?php
require_once( '../phttapi.php' );

if ( array_key_exists( 'session_id', $_REQUEST ) ) {
    session_id( $_REQUEST['session_id'] );
}
session_start();

if ( array_key_exists( 'exiting', $_REQUEST ) ) {
    header( 'Content-Type: text/plain' );
    print "OK";
    exit();
}

header( 'Content-Type: text/xml' );

if ( array_key_exists( 'ivr_path', $_REQUEST ) ) {
    $ivr_path = $_REQUEST['ivr_path'];
} else {
    $ivr_path = 'main_menu';
}

if ( file_exists( "$ivr_path.php" ) ) {
    include( "$ivr_path.php" );
} else {
    $api = new phttapi();
    $api->start_work();
    $h   = new phttapi_hangup();
    $h->cause( 'NORMAL_CLEARING' );
    $api->add_action( $h );
    $api->end_work();
    print $api->output();
}






