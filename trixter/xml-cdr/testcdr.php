<?php  //  -*- mode:c; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil;  -*-
include_once('Parse_XML_CDR.php');
Header("Content-type: text/plain");

function read_file($filename)
{
    if ( file_exists($filename) && ($fd = @fopen($filename, 'rb')) ) {
        $contents = '';
        while (!feof($fd)) {
            $contents .= fread($fd, 8192);
        }
        fclose($fd);
        return $contents;
    } else {
        return false;
    }
}

$xmlraw=read_file("testcdr.xml");
if($xmlraw !== false) {
    $xml = new Parse_CDR_XML($xmlraw);
    $cdr=$xml->ReturnArray();
    print_r($cdr);
 } else {
    echo "Unable to read file";
 }
?>
