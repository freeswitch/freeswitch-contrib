<?php

include( 'login.php' );

$api = new phttapi();
$api->start_work();

$b   = new phttapi_action_binding( '~\d+#' );
$b->strip('#');

$pb  = new phttapi_playback();
$pb->file( sprintf( 'http://%s%s/exten.wav', $_SERVER['HTTP_HOST'], dirname( $_SERVER['PHP_SELF'] ) ) );
$pb->error_file( sprintf( 'http://%s%s/invalid.wav', $_SERVER['HTTP_HOST'], dirname( $_SERVER['PHP_SELF'] ) ) );
$pb->input_timeout( '15000' );
$pb->digit_timeout( '5000' );
$pb->action( 'dial:disa:XML' );
$pb->add_binding($b);

$api->add_action($pb);
$api->end_work();
print $api->output();

