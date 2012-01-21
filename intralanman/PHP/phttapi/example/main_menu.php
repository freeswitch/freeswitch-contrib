<?php

if ( array_key_exists( 'option', $_REQUEST ) ) {
  $option = $_REQUEST['option'];
} else {
  $option = '';
}

if ( $option ) {
  switch ( $option ) {
  case '1':
    $menu_ivr_path = 'disa';
    break;
  case '2':
    $menu_ivr_path = 'record';
    break;
    //  case '':
    //    $menu_ivr_path = '';
    //    break;
  }

  $api = new phttapi();

  $api->start_variables();
  $api->add_variable( 'ivr_path', $menu_ivr_path );
  $api->end_variables();

  $api->start_work();

  $var = new phttapi_getVariable( 'ivr_path' );
  $var->permanent( 'true' );
  $api->add_action( $var );

  $api->end_work();
  print $api->output();

} else {
  $api = new phttapi();
  $api->start_work();

  $bind1 = new phttapi_action_binding( '1' );
  $bind2 = new phttapi_action_binding( '2' );

  $play = new phttapi_speak( "to make an outbound call, press 1,, to record a file, press 2,, " );
  $play->engine( 'flite' );
  $play->voice( 'slt' );
  $play->name( 'option' );
  $play->input_timeout( '5000' );
  $play->add_binding($bind1);
  $play->add_binding($bind2);

  $api->add_action( $play );
  $api->end_work();
  print $api->output();

}


