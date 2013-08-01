<?php

	/**
	 * @todo   add unit tests for all methods in all classes
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

	/**
	 *
	 * @author Raymond Chandler (intralanman@gmail.com)
	 *
	 */
	class MissingRequiredAttribute extends Exception {

	}

	class phttapi {

		private $logptr;
		public $xml;
		private $work_started = FALSE;
		private $work_ended = FALSE;

		public function __construct() {
			$this->logptr = fopen( '/tmp/phttapi.log', 'a' );
			$this->dlog( print_r( $_REQUEST, TRUE ) );

			$this->xml = new XMLWriter();
			$this->xml->openMemory();

			$this->xml->setIndent( TRUE );
			$this->xml->setIndentString( "  " );
			$this->open( 'document' );
			$this->attr( 'type', 'text/freeswitch-httapi' );
		}

		public function output() {
			if ( $this->work_started && !$this->work_ended ) {
				$this->end_work();
			}
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

		public function dlog( $debug = NULL ) {
			fwrite( $this->logptr, $debug );
		}

		public function start_work() {
			$this->work_started = TRUE;
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
			if ( !$this->work_started ) {
				$this->start_work();
			}
			if ( is_object( $action ) && $action instanceof phttapi_action ) {
				if ( property_exists( $action, 'required_attributes' ) && is_array( $action->required_attributes ) ) {
					foreach ( $action->required_attributes as $req_key => $req_val ) {
						if ( isset( $req_val ) && !array_key_exists( $req_key, $action->defined_attributes ) ) {
							throw new MissingRequiredAttribute( "$req_key is required" );
						}
					}

				}

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

		public $action_text = NULL;
		public $action_name = NULL;
		public $attributes = array();
		public $defined_attributes = array();
		public $required_attributes = array();
		public $global_attributes = array(
			'action'      => TRUE,
			'temp-action' => TRUE
		);

		public function __call( $method, $args ) {
			$attribute = preg_replace( '/_/', '-', $method );

			$this->attributes = array_merge( $this->attributes, $this->global_attributes );

			if ( property_exists( $this, 'attributes' )
			     && is_array( $this->attributes )
			     && array_key_exists( $attribute, $this->attributes )
			     && !empty( $this->attributes [$attribute] )
			) {
				if ( !count( $args ) ) {
					throw new NullDataException( "$method requires an argument" );
				}
				$value = $args [0];
				$this->attr( $attribute, $value );
			} else {
				if ( !property_exists( $this, 'attributes' ) ) {
					trigger_error( 'no "attributes" property' );
				} elseif ( !is_array( $this->attributes ) ) {
					trigger_error( 'attributes property is not an array' );
				} elseif ( !array_key_exists( $attribute, $this->attributes ) ) {
					trigger_error( "\"$attribute\" is not in the attributes array" );
				} else {
					trigger_error( "\"$attribute\" seems to be empty" );
				}
				throw new NotImplementedException( sprintf( "%s doesn't implement %s", get_class( $this ), $method ) );
			}
		}

		public function attr( $attr_name, $attr_value ) {
			$this->defined_attributes [$attr_name] = $attr_value;
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

		public function __construct( $text = NULL ) {
			if ( !$text ) {
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

	/**
	 *
	 *
	 * @method void digit_timeout
	 * @method void input_timeout
	 * @method void error_file
	 * @method void terminators
	 */
	class phttapi_prompt extends phttapi_action {

		public $bindings = array();

		public function add_binding( $binding ) {
			if ( is_object( $binding ) && $binding instanceof phttapi_action_binding ) {
				array_push( $this->bindings, $binding );
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

		public $attributes = array(
			'profile' => TRUE
		);

		public function __construct( $text = NULL ) {
			if ( !$text ) {
				throw new NullDataException( "no data passed" );
			}
			$this->action_text = $text;
		}

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

		public $attributes = array(
			'context'          => TRUE,
			'dialplan'         => TRUE,
			'caller-id-name'   => TRUE,
			'caller-id-number' => TRUE
		);

		public function __construct( $text = NULL ) {
			if ( !$text ) {
				throw new NullDataException( "no data passed" );
			}
			$this->action_text = $text;
		}

	}

	/**
	 * @method void application
     * @data void data
	 *
	 * @author Raymond Chandler (intralanman@gmail.com)
	 *
	 */
	class phttapi_execute extends phttapi_action {

		public $attributes = array(
			'application' => TRUE,
			'data'        => TRUE,
		);

		public function __construct( $data = NULL ) {
			if ( $data ) {
				$this->action_text = $data;
			}
		}
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

		public $attributes = array(
			'permanent' => TRUE
		);

		public function __construct( $var = NULL ) {
			if ( !$var ) {
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

		public $attributes = array(
			'cause' => TRUE
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

		public $attributes = array(
			'level' => TRUE,
			'clean' => TRUE
		);

		public function __construct( $text = NULL ) {
			if ( !$text ) {
				throw new NullDataException( 'match digits must be passed' );
			}

			$text              = trim( $text );
			$this->action_text = "$text\n";
		}
	}

	/**
	 * @method void milliseconds
	 * @method void error_file
	 * @method void digit_timeout
	 *
	 * @author Raymond Chandler (intralanman@gmail.com)
	 *
	 */
	class phttapi_pause extends phttapi_prompt {

		public $attributes = array(
			'terminators' => TRUE,
			'milliseconds'  => TRUE,
			'error-file'    => TRUE,
			'digit-timeout' => TRUE,
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

		public $attributes = array(
			'file'          => TRUE,
			'error-file'    => TRUE,
			'digit-timeout' => TRUE,
			'input-timeout' => TRUE,
			'loops'         => TRUE,
			'name'          => TRUE,
			'asr-engine'    => TRUE,
			'asr-grammar'   => TRUE,
			'terminators' => TRUE,
		);

	}

	/**
	 * @method void file
	 * @method void error_file
	 * @method void digit_timeout
	 * @method void input_timeout
	 * @method void loops
	 * @method void beep_file
	 *
	 * @author Raymond Chandler (intralanman@gmail.com)
	 *
	 */
	class phttapi_record extends phttapi_prompt {

		public $attributes = array(
			'beep-file'     => TRUE,
			'file'          => TRUE,
			'error-file'    => TRUE,
			'digit-timeout' => TRUE,
			'input-timeout' => TRUE,
			'limit'         => TRUE,
			'terminators' => TRUE,
		);

	}

	/**
	 * @method void limit
	 *
	 * @author Raymond Chandler (intralanman@gmail.com)
	 *
	 */
	class phttapi_recordCall extends phttapi_action {

		public $attributes = array(
			'limit' => TRUE
		);

	}

	/**
	 * @method void text
	 * @method void gender
	 * @method void method
	 * @method void type
	 * @method void language
	 *
	 * @author Raymond Chandler (intralanman@gmail.com)
	 *
	 */
	class phttapi_say extends phttapi_prompt {

		public $attributes = array(
			'text'          => TRUE,
			'gender'        => TRUE,
			'method'        => TRUE,
			'type'          => TRUE,
			'language'      => TRUE,
			'error-file'    => TRUE,
			'input-timeout' => TRUE,
			'digit-timeout' => TRUE,
			'terminators' => TRUE,
		);

		public $required_attributes = array(
			'language' => TRUE,
			'type'     => TRUE,
			'method'   => TRUE,
			'text'     => TRUE,
		);

		public function __construct( $text = NULL ) {
			if ( $text ) {
				$this->action_text = $text;
			}
		}

	}

	/**
	 * @method void engine
	 * @method void voice
	 * @method void loops
	 * @method void text
	 *
	 * @author Raymond Chandler (intralanman@gmail.com)
	 *
	 */
	class phttapi_speak extends phttapi_prompt {

		public $attributes = array(
			'engine'        => TRUE,
			'voice'         => TRUE,
			'digit-timeout' => TRUE,
			'input-timeout' => TRUE,
			'error-file'    => TRUE,
			'loops'         => TRUE,
			'text'          => TRUE,
			'terminators' => TRUE,
		);

		public $required_attributes = array(
			'engine' => TRUE,
			'voice'  => TRUE,
			'text'   => TRUE,
		);
	}

	/**
	 * @method void to
	 *
	 * @author Raymond Chandler (intralanman@gmail.com)
	 *
	 */
	class phttapi_sms extends phttapi_action {

		public $attributes = array(
			'to' => TRUE,
		);

		public $required_attributes = array(
			'to' => TRUE,
		);

		public function __construct( $text = NULL ) {
			if ( !$text ) {
				throw new Exception( "no message entered" );
			}
			$this->action_text = $text;
		}

	}

	/**
	 * @method void id()
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
	class phttapi_vmname extends phttapi_prompt {

		public $attributes = array(
			'id'            => TRUE,
			'name'          => TRUE,
			'error-file'    => TRUE,
			'digit-timeout' => TRUE,
			'input-timeout' => TRUE,
			'loops'         => TRUE,
			'terminators' => TRUE,
		);

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

		public $attributes = array(
			'auth-only' => TRUE,
			'domain'    => TRUE,
			'profile'   => TRUE,
			'check'     => TRUE,
			'id'        => TRUE
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

		private $lang = 'en';

		public function __construct() {
			$this->action_name = 'playback';
		}

		public function lang( $lang ) {
			$this->lang = $lang;
		}

		public function text( $text ) {
			$lang = $this->lang;
			$text = preg_replace( '/\s/', '+', $text );

			$url = "http://(ext=mp3,user_agent='Mozilla/5.0 (X11; Linux x86_64; rv:9.0.1) Gecko/20100101 Firefox/9.0.1')translate.google.com/translate_tts?tl=$lang&q=$text";
			$this->file( $url );
		}

	}

