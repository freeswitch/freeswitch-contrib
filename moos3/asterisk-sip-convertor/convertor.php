<?php

$file = 'sip-users.example';
$vfile = 'voicemail.example';
$users = parse_ini_file($file, true);
$lines = file($vfile);
$vmboxs = convert_aster_voicemail($lines);
$template = file_get_contents('Templates/User-Template.xml');
echo "Converting Asterisk Users to user files".PHP_EOL;
echo "Using Template: Templates/User-Template.xml".PHP_EOL;
echo "".PHP_EOL;
echo "Running..... ".PHP_EOL;

foreach ($users as $extension => $values){
	makeUser($template, $extension, $values['secret'], $values['context'], $values['callerid'], $vmboxs[$extension]);
}

echo "Completed Run".PHP_EOL;

/***

Processes voicemail.conf and makes it so you can set the voicemail password, full name and email from voicemail.conf 
to the user xml files based off extension.

**/

function convert_aster_voicemail($lines){
	$data = array();
	foreach($lines as &$line){
		$line = preg_split('/\s*=>\s*/', trim($line));
		if (is_numeric($line[0])){
			$data[$line[0]] = explode(',',$line[1]);
		}
	}
	return $data;
}
//Processes template and populates it with values from the sip_peers file and information from the voicemail.conf files

function makeUser($template, $extension, $password, $context, $callerid, $vmbox) {
	$xml = str_replace('[[[username]]]', $extension, $template);
	$xml = str_replace('[[[password]]]', $password, $xml);
	$xml = str_replace('[[[extension]]]', $extension, $xml);
	$xml = str_replace('[[[callerid]]]', $callerid, $xml);
	$xml = str_replace('[[[context]]]', $context, $xml);
	if (empty($vmbox[1])){
		$xml = str_replace('[[[disable_directory]]]','<param name="directory-exten-visible" value="false"/>',$xml);
		$xml = str_replace('[[[full_name]]]','' ,$xml);
	
	} else {
		$xml = str_replace('[[[disable_directory]]]','' , $xml);
		$full_name = "<variable name=\"directory_full_name\" value=\"$vmbox[1]\"/>";
		$xml = str_replace('[[[full_name]]]',$full_name ,$xml);
	}
	$xml = str_replace('[[[email]]]', $vmbox[2], $xml);
	if (empty($vmbox[0])){
		$xml = str_replace('[[[vm_pass]]]', $extension, $xml);
	} else {
		$xml = str_replace('[[[vm_pass]]]', $vmbox[0], $xml);
	}
	echo "Writing XML file for $extension".PHP_EOL;
	file_put_contents('extensions/'.$extension.'.xml', $xml);
}

?>
