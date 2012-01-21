<?php
class NotImplementedException extends Exception {}
class NullDataException extends Exception {}

class phttapi_xml_blob {
  public $xml;

  public function __construct() {
    $this->xml = new XMLWriter();
    $this->xml->openMemory();
  }

  public function output() {
    $this->xml->endElement();
    $xml = $this->xml->outputMemory();

    return $xml;
  }

  public function open( $tag ) {
    $this->xml->startElement( $tag );
  }

  public function text( $text ) {
    $this->xml->text( $text );
  }

  public function close() {
    $this->xml->endElement();
  }

  public function attr( $key, $val ) {
    $this->xml->writeAttribute( $key, $val );
  }

  public function raw( $xml ) {
    $this->xml->writeRaw( $xml );
  }
}

class phttapi extends phttapi_xml_blob {
  private $logptr;

  public function __construct() {
    $this->logptr = fopen( '/tmp/phttapi.log', 'a' );
    $this->dlog( print_r( $_REQUEST, true ) );

    parent::__construct();

    $this->xml->setIndent( true );
    $this->xml->setIndentString( "  " );
    $this->open( 'document' );
    $this->attr( 'type', 'text/freeswitch-httapi' );
  }

  public function dlog( $debug = null ) {
    fwrite( $this->logptr, $debug );
  }

  public function output() {
    $xml = parent::output();
    $this->dlog( $xml );    
    return $xml;
  }

  public function start_work() {
    $this->open( 'work' );
  }

  public function end_work() {
    $this->close();
  }

  public function start_variables() {
    $this->open( 'variables' );
  }

  public function add_variable( $var, $val ) {
    $this->open( $var );
    $this->text( $val );
    $this->close();
  }

  public function end_variables() {
    $this->close();
  }

  public function start_params() {
    $this->open( 'params' );
  }

  public function add_param( $param, $value ) {
    $this->open( $param );
    $this->text( $value );
    $this->close();
  }

  public function end_params() {
    $this->close();
  }

  public function add_action( $action ) {
    $xml = $action->output();

    $this->raw( $xml );
  }
}

class phttapi_action extends phttapi_xml_blob {
  private $action_text = null;

  public function __call( $method, $args ) { 
    $attribute = preg_replace( '/_/', '-', $method);
    if ( property_exists($this, 'attributes') && is_array( $this->attributes ) && array_key_exists( $attribute, $this->attributes ) && !empty( $this->attributes[$attribute] ) ) {
      if ( !count( $args ) ) {
	throw new NullDataException("$method requires an argument");
      }
      $value = $args[0];
      $this->attr( $attribute, $value );
    } else {
      throw new NotImplementedException( sprintf( "%s doesn't implement %s", get_class( $this ), $method ) ); 
    }
  }

  public function __construct( $text = null ) {
    parent::__construct();
    $this->open( preg_replace( '/^phttapi_/', '', get_class( $this ) ) );
    $this->action_text = $text;
  }

  public function action( $value ) {
    $this->attr( preg_replace( '/_/', '-', __FUNCTION__ ), $value );
  }

  public function temp_action( $value ) {
    $this->attr( preg_replace( '/_/', '-', __FUNCTION__ ), $value );
  }

  public function name( $value ) {
    $this->attr( preg_replace( '/_/', '-', __FUNCTION__ ), $value );
  }

  public function output() {
    if ( $this->action_text ) {
      $this->text( $this->action_text );
    }

    return parent::output();
  }
}

class phttapi_action_binding extends phttapi_xml_blob{
  public $action_text;

  public function __construct( $text = null ) {
    if ( !$text ) throw new NullDataException( 'match digits must be passed' );

    parent::__construct();

    $this->open( 'bind' );
    $this->action_text = $text;
  }

  public function strip( $value ) {
    $this->attr( preg_replace( '/_/', '-', __FUNCTION__ ), $value );
  }

  public function output() {
    $this->text( $this->action_text );
    return parent::output();
  }
}

class phttapi_prompt extends phttapi_action {
  public function add_binding( $binding ) {
    $xml = $binding->output();

    if ( get_class( $binding ) == 'phttapi_action_binding' ) {
      $this->raw( $xml );
    }
  }
}


// ACTIONS
class phttapi_break      extends phttapi_action {}

class phttapi_conference extends phttapi_action {
  public $attributes = array(
			     'profile' => true,
			     );
}

class phttapi_continue   extends phttapi_action {}

class phttapi_dial       extends phttapi_action {
  public $attributes = array(
			     'context'          => true,
			     'dialplan'         => true,
			     'caller_id_name'   => true,
			     'caller_id_number' => true,
			     );

  public function __construct( $text = null ) { if ( !$text ) throw new NullDataException("no data passed"); }
}

class phttapi_execute    extends phttapi_action {
  public $attributes = array(
			     'application' => true,
			     );
}
if ( class_exists( 'PHPUnit_Framework_TestCase' ) ) {
  class phttapi_executeTest extends PHPUnit_Framework_TestCase {
    /**
     * @expectedException NullDataException
     */
    public function testNoData() {
      $e = new phttapi_execute();
    }
    /**
     * @expectedException NotImplementedException
     */
    public function testFileThrowsException() {
      $e = new phttapi_execute( 'text' );
      $e->file();
    }
  }
}

class phttapi_getVariable extends phttapi_action {
  public $attributes = array(
			     'permanent' => true,
			     );

  public function __construct( $var = null ) {
    if ( !$var ) {
      throw new NullDataException( "getVariable must be instantiated with a variable name" );
    } else {
      parent::__construct();
      $this->attr( 'name', $var );
    }
  }
}

class phttapi_hangup     extends phttapi_action {
  public $attributes = array(
			     'cause' => true,
			     );
}

class phttapi_log        extends phttapi_action {
  public $attributes = array(
			     'level' => true,
			     'clean' => true,
			     );
}

class phttapi_pause      extends phttapi_action {
  public $attributes = array(
			     'milliseconds' => true,
			     );
}

class phttapi_playback   extends phttapi_prompt {
  public $attributes = array(
			     'file'          => true,
			     'error-file'    => true,
			     'digit-timeout' => true,
			     'input-timeout' => true,
			     'loops'         => true,
			     'asr-engine'    => true,
			     'asr-grammar'   => true,
			     );
}

class phttapi_record     extends phttapi_prompt {
  public $attributes = array(
			     'file'          => true,
			     'error-file'    => true,
			     'digit-timeout' => true,
			     'input-timeout' => true,
			     'loops'         => true,
			     );
}

class phttapi_recordCall extends phttapi_action {
  public $attributes = array(
			     'limit' => true,
			     );
}

class phttapi_say        extends phttapi_prompt {
  public $attributes = array(
			     'gender'   => true,
			     'method'   => true,
			     'type'     => true,
			     'language' => true,
			     );
}

class phttapi_speak      extends phttapi_prompt {
  public $attributes = array(
			     'engine'        => true,
			     'voice'         => true,
			     'digit-timeout' => true,
			     'input-timeout' => true,
			     'loops'         => true,
			     );
}

class phttapi_sms        extends phttapi_action {
  public $attributes = array(
			     'to' => true,
			     );

  public function __construct( $text = null ) { if ( !$text ) throw new Exception("no data passed"); }
}

class phttapi_voicemail  extends phttapi_action {
  public $attributes = array(
			     'auth-only' => true,
			     'domain'    => true,
			     'profile'   => true,
			     'check'     => true,
			     'id'        => true,
			     );
}



  /*
  */

