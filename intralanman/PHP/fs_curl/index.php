<?php
/**
 * @package FS_CURL
 * @license BSD
 * @author Raymond Chandler (intralanman) <intralanman@gmail.com>
 * @version 0.1
 * initial page hit in all curl requests
 */

/**
 * define for the time that execution of the script started
 */
define('START_TIME', ereg_replace('^0\.([0-9]+) ([0-9]+)$', '\2.\1', microtime()));

/**
 * Pre-Class initialization die function
 * This function should be called on any
 * critical error condition before the fs_curl
 * class is successfully instantiated.
 * @return void
*/

function file_not_found($no=false, $str=false, $file=false, $line=false) {
    if ($no == E_STRICT) {
        return;
    }
    header('Content-Type: text/xml');
    $xmlw = new XMLWriter();
    $xmlw -> openMemory();
    $xmlw -> setIndent(true);
    $xmlw -> setIndentString('  ');
    $xmlw -> startDocument('1.0', 'UTF-8', 'no');
    //set the freeswitch document type
    $xmlw -> startElement('document');
    $xmlw -> writeAttribute('type', 'freeswitch/xml');
    $xmlw -> startElement('section');
    $xmlw -> writeAttribute('name', 'result');
    $xmlw -> startElement('result');
    $xmlw -> writeAttribute('status', 'not found');
    $xmlw -> endElement();
    $xmlw -> endElement();
    $xmlw -> endElement();
    if (!empty($no) && !empty($str) && !empty($file) &&!empty($line)) {
        $xmlw -> writeComment("ERROR: $no - ($str) on line $line of $file");
    }
    echo $xmlw -> outputMemory();
    exit();
}
error_reporting(E_ALL);
set_error_handler('file_not_found');

if (!class_exists('XMLWriter')) {
    trigger_error(
    "XMLWriter Class NOT Found... You Must install it before using this package"
        , E_USER_ERROR
    );
}
if (!(@include_once('fs_curl.php'))
    || !(@include_once('global_defines.php'))) {
    trigger_error(
    'could not include fs_curl.php or global_defines.php', E_USER_ERROR
    );
}
if (!is_array($_REQUEST)) {
    trigger_error('$_REQUEST is not an array');
}
$section = $_REQUEST['section'];
$section_file = sprintf('fs_%s.php', $section);
/**
 * this include will differ based on the section that's passed
 */
if (!(@include_once($section_file))) {
    trigger_error("unable to include $section_file");
}
switch ($section) {
    case 'configuration':
        if (!array_key_exists('key_value', $_REQUEST)) {
            trigger_error('key_value does not exist in $_REQUEST');
        }
        $config = $_REQUEST['key_value'];
        $processor = sprintf('configuration/%s.php', $config);
        $class = str_replace('.', '_', $config);
        if (!(@include_once($processor))) {
            trigger_error("unable to include $processor");
        }
        $conf = new $class;
        $conf -> comment("class name is $class");
        break;
    case 'dialplan':
        $conf = new fs_dialplan();
        break;
    case 'directory':
        $conf = new fs_directory();
        break;
}

$conf -> debug('---- Start _REQUEST ----');
$conf -> debug($_REQUEST);
$conf -> debug('---- End _REQUEST ----');
$conf -> main();
$conf -> output_xml();

?>
