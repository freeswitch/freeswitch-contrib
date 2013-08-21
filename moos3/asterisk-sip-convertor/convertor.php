<?php

$file = 'sip-users.example';
$users = parse_ini_file($file, true);
$template = file_get_contents('Templates/User-Template.xml');
echo "Converting Asterisk Users to user files".PHP_EOL;
echo "Using Template: Templates/User-Template.xml".PHP_EOL;
echo "".PHP_EOL;
echo "Running..... ".PHP_EOL;

foreach ($users as $extension => $values){
	makeUser($template, $extension, $values['secret'], $values['context'], $values['callerid']);
}

echo "Completed Run".PHP_EOL;

function makeUser($template, $extension, $password, $context, $callerid) {
	$xml = str_replace('[[[username]]]', $extension, $template);
	$xml = str_replace('[[[password]]]', $password, $xml);
	$xml = str_replace('[[[extension]]]', $extension, $xml);
	$xml = str_replace('[[[callerid]]]', $callerid, $xml);
	$xml = str_replace('[[[context]]]', $context, $xml);
	file_put_contents('extensions/'.$extension.'.xml', $xml);
}

?>
