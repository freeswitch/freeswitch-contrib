<?php
if ( !array_key_exists( 'authenticated', $_SESSION ) || $_SESSION['authenticated'] != 'true' ) {
  if ( array_key_exists( 'user_pin_authenticated', $_REQUEST ) ) {
    $_SESSION['authenticated'] = true;
    $_SESSION['authenticated_user'] = $_REQUEST['user_pin_authenticated_user'];

    $api = new phttapi();
    $api->start_variables();
    $api->add_variable('authenticated_user', $_REQUEST['user_pin_authenticated_user'] );
    $api->end_variables();

    unset( $_SESSION['authenticating'] );
  } elseif ( array_key_exists( 'authenticating', $_SESSION ) ) {
    $class     = new phttapi();
    $class->start_work();
    
    $authed    = new phttapi_getVariable( 'user_pin_authenticated' );
    $class->add_action( $authed );

    $auth_user = new phttapi_getVariable( 'user_pin_authenticated_user' );
    $class->add_action( $auth_user );
    
    $class->end_work();
    print $class->output();

    exit();
  } else {
    $_SESSION['authenticating'] = true;

    $class = new phttapi();
    $class->start_work();
    
    $vm = new phttapi_voicemail();
    $vm->auth_only( 'true' );
    $vm->domain( 'home.intralanman.com' );
    $class->add_action( $vm );
    
    $class->end_work();
    print $class->output();

    exit();
  }
}

