<?php
/**
 * @package FS_CURL
 * @license
 * @author Raymond Chandler (intralanman) <intralanman@gmail.com>
 * @version 0.1
 * initial page hit in all curl requests
 */

/**
 * Pre-Class initialization die function
 * This function should be called on any
 * critical error condition before the fs_curl
 * class is successfully instantiated.
 * @return void
*/
function file_not_found($no=false, $str=false, $file=false, $line=false) {
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
    echo $xmlw -> outputMemory();
    exit();
}

if (!(@include_once('fs_curl.php'))
|| !(@include_once('global_defines.php'))) {
    file_not_found();
}
$section = $_REQUEST['section'];
$section_file = sprintf('fs_%s.php', $section);
/**
 * this include will differ based on the section that's passed
 */
if (!(@include_once($section_file))) {
    file_not_found();
}
switch ($section) {
    case 'configuration':
        $config = $_REQUEST['key_value'];
        $processor = sprintf('configuration/%s.php', $config);
        $class = str_replace('.', '_', $config);
        if (!(@include_once($processor))) {
            file_not_found();
            exit();
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
$conf -> main();
$conf -> output_xml();
?>