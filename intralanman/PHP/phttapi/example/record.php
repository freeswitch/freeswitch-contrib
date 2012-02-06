<?php

require_once( 'login.php' );

$api = new phttapi();
$api->start_work();
$h = new phttapi_hangup(); 

if ( is_array( $_FILES ) ) {
  $api->add_action( $h );
} else {
  $api->add_action( $h );
}

$api->end_work();
print $api->output();