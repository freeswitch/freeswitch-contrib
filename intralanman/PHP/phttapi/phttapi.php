<?php
/**
 * @todo add unit tests for all methods in all classes
 * @author Raymond Chandler (intralanman@gmail.com)
 *
 */

/**
 *
 * @author Raymond Chandler (intralanman@gmail.com)
 *
 */
 class NotImplementedException extends Exception {
}

/**
 *
 * @author Raymond Chandler (intralanman@gmail.com)
 *
 */
class NullDataException extends Exception {
}
class MissingRequiredAttribute extends Exception {
}

class phttapi {
	private $logptr;
	public $xml;

	public function __construct() {
		$this->logptr = fopen( '/tmp/phttapi.log', 'a' );
		$this->dlog( print_r( $_REQUEST, true ) );

		$this->xml = new XMLWriter();
		$this->xml->openMemory();

		$this->xml->setIndent( true );
		$this->xml->setIndentString( "  " );
		$this->open( 'document' );
		$this->attr( 'type', 'text/freeswitch-httapi' );
	}

	public function output() {
		$this->xml->endElement(); // </document>
		$xml = $this->xml->outputMemory();

		$this->dlog( $xml );
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

	public function comment( $comment ) {
		$this->xml->writeComment( $comment );
	}

	public function dlog( $debug = null ) {
		fwrite( $this->logptr, $debug );
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
		if ( is_object( $action ) && $action instanceof phttapi_action ) {
			if ( property_exists( $action, 'action_name' ) && $action->action_name ) {
				$action_name = $action->action_name;
			} else {
				$action_name = preg_replace( '/^phttapi_/', '', get_class( $action ) );
			}

			$this->open( $action_name );

			foreach ( $action->defined_attributes as $attr_name => $attr_value ) {
				$this->attr( $attr_name, $attr_value );
			}

			if ( property_exists( $action, 'bindings' ) ) {
				foreach ( $action->bindings as $binding ) {
					$this->open( 'bind' );
					if ( $binding->strip ) {
						$this->attr( 'strip', $binding->strip );
					}
					$this->text( $binding->match );
					$this->close();
				}
			}

			if ( $action->action_text ) {
				$this->text( $action->action_text );
			}
			$this->close();
		}
	}
}

/**
 * @method void action
 * @method void temp_action
 * @author Raymond Chandler (intralanman@gmail.com)
 *
 */
class phttapi_action {
	public $action_text = null;
	public $action_name = null;
	public $attributes = array ();
	public $defined_attributes = array ();
	public $required_attributes = array ();

	public function __call( $method, $args ) {
		$attribute = preg_replace( '/_/', '-', $method );
		if ( property_exists( $this, 'attributes' ) && is_array( $this->attributes ) && array_key_exists( $attribute, $this->attributes ) && ! empty( $this->attributes [$attribute] ) ) {
			if ( ! count( $args ) ) {
				throw new NullDataException( "$method requires an argument" );
			}
			$value = $args [0];
			$this->attr( $attribute, $value );
		} else {
			throw new NotImplementedException( sprintf( "%s doesn't implement %s", get_class( $this ), $method ) );
		}
	}

	public function attr( $attr_name, $attr_value ) {
		$this->defined_attributes [$attr_name] = $attr_value;
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
	}
}

class phttapi_action_binding {
	public $match;
	public $strip;

	public function __construct( $text = null ) {
		if ( ! $text ) {
			throw new NullDataException( 'match digits must be passed' );
		}

		$this->match = $text;
	}

	public function strip( $value = NULL ) {
		if ( $value ) {
			$this->strip = $value;
		} else {
			return $this->strip;
		}
	}

}

class phttapi_prompt extends phttapi_action {
	public $bindings = array ();

	public function add_binding( $binding ) {
		if ( is_object( $binding ) && $binding instanceof phttapi_action_binding ) {
			array_push( $this->bindings, &$binding );
		}
	}
}

// ACTIONS
class phttapi_break extends phttapi_action {
}

/**
 * @method void profile
 *
 * @author Raymond Chandler (intralanman@gmail.com)
 *
 */
class phttapi_conference extends phttapi_action {
	public $attributes = array (
			'profile' => true
	);
}

class phttapi_continue extends phttapi_action {
}

/**
 * @method void context
 * @method void dialplan
 * @method void caller_id_name
 * @method void caller_id_number
 *
 * @author Raymond Chandler (intralanman@gmail.com)
 *
 */
class phttapi_dial extends phttapi_action {
	public $attributes = array (
			'context' => true,
			'dialplan' => true,
			'caller_id_name' => true,
			'caller_id_number' => true
	);

	public function __construct( $text = null ) {
		if ( ! $text ) {
			throw new NullDataException( "no data passed" );
		}
	}
}

/**
 * @method void application
 *
 * @author Raymond Chandler (intralanman@gmail.com)
 *
 */
class phttapi_execute extends phttapi_action {
	public $attributes = array (
			'application' => true
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

/**
 * @method void permanent
 * @author Raymond Chandler (intralanman@gmail.com)
 *
 */
class phttapi_getVariable extends phttapi_action {
	public $attributes = array (
			'permanent' => true
	);

	public function __construct( $var = null ) {
		if ( ! $var ) {
			throw new NullDataException( "getVariable must be instantiated with a variable name" );
		} else {
			$this->attr( 'name', $var );
		}
	}
}

/**
 * @method void cause
 *
 * @author Raymond Chandler (intralanman@gmail.com)
 *
 */
class phttapi_hangup extends phttapi_action {
	public $attributes = array (
			'cause' => true
	);
}

/**
 * @method void level
 * @method void clean
 *
 * @author Raymond Chandler (intralanman@gmail.com)
 *
 */
class phttapi_log extends phttapi_action {
	public $attributes = array (
			'level' => true,
			'clean' => true
	);
}

/**
 * @method void milliseconds
 *
 * @author Raymond Chandler (intralanman@gmail.com)
 *
 */
class phttapi_pause extends phttapi_action {
	public $attributes = array (
			'milliseconds' => true
	);
}

/**
 * @method void file()
 * @method void error_file()
 * @method void digit_timeout()
 * @method void input_timeout()
 * @method void loops()
 * @method void name()
 * @method void asr_engine()
 * @method void asr_grammar()
 *
 * @author Raymond Chandler (intralanman@gmail.com)
 *
 */
class phttapi_playback extends phttapi_prompt {
	public $attributes = array (
			'file' => true,
			'error-file' => true,
			'digit-timeout' => true,
			'input-timeout' => true,
			'loops' => true,
			'name' => true,
			'asr-engine' => true,
			'asr-grammar' => true
	);
}

/**
 * @method void file
 * @method void error_file
 * @method void digit_timeout
 * @method void input_timeout
 * @method void loops
 *
 * @author Raymond Chandler (intralanman@gmail.com)
 *
 */
class phttapi_record extends phttapi_prompt {
	public $attributes = array (
			'file' => true,
			'error-file' => true,
			'digit-timeout' => true,
			'input-timeout' => true,
			'loops' => true
	);
}

/**
 * @method void limit
 *
 * @author Raymond Chandler (intralanman@gmail.com)
 *
 */
class phttapi_recordCall extends phttapi_action {
	public $attributes = array (
			'limit' => true
	);
}

/**
 * @method void gender
 * @method void method
 * @method void type
 * @method void language
 *
 * @author Raymond Chandler (intralanman@gmail.com)
 *
 */
class phttapi_say extends phttapi_prompt {
	public $attributes = array (
			'gender' => true,
			'method' => true,
			'type' => true,
			'language' => true
	);
}

/**
 * @method void engine
 * @method void voice
 * @method void digit_timeout
 * @method void input_timeout
 * @method void loops
 *
 * @author Raymond Chandler (intralanman@gmail.com)
 *
 */
class phttapi_speak extends phttapi_prompt {
	public $attributes = array (
			'engine' => true,
			'voice' => true,
			'digit-timeout' => true,
			'input-timeout' => true,
			'loops' => true
	);
}

/**
 * @method void to
 *
 * @author Raymond Chandler (intralanman@gmail.com)
 *
 */
class phttapi_sms extends phttapi_action {
	public $attributes = array (
			'to' => true
	);

	public function __construct( $text = null ) {
		if ( ! $text ) {
			throw new Exception( "no data passed" );
		}
	}
}

/**
 * @method void auth_only
 * @method void domain
 * @method void profile
 * @method void check
 * @method void id
 *
 * @author Raymond Chandler (intralanman@gmail.com)
 *
 */
class phttapi_voicemail extends phttapi_action {
	public $attributes = array (
			'auth-only' => true,
			'domain' => true,
			'profile' => true,
			'check' => true,
			'id' => true
	);
}

/**
 * phttapi extensions - classes that aren't actually part of the httapi spec,
 * but prove to be pretty useful
 */



/**
 * phttapi_google_tts offers a simple way to use Google's TTS api to pull a sound file to be played
 *
 * @author Raymond Chandler (intralanman@gmail.com)
 *
 */
class phttapi_google_tts extends phttapi_playback {

	public function __construct() {
		$this->action_name = 'playback';
	}

	public function text( $text ) {
		$text = preg_replace( '/\s/', '+', $text );
		$url = "http://(ext=mp3,user_agent='Mozilla/5.0 (X11; Linux x86_64; rv:9.0.1) Gecko/20100101 Firefox/9.0.1')translate.google.com/translate_tts?q=$text";
		$this->file( $url );
	}
}

