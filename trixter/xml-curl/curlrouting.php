<?php  //  -*- mode:c; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil;  -*-
Header("Content-type: text/plain");

$callBacks = Array();
ini_set("magic_quotes_gpc","Off");
include_once('Smarty.class.php');

// load the modules
$dh = @opendir("./modules") or die("Unable to open modules dir");
while ($file = readdir($dh)) {
    if(preg_match("/.*\.php$/i",$file)) {
        include_once("modules/$file");
    }
}


if(isset($_POST['section'])) {
    if(isset($callBacks[$_POST['section']])) {
        call_user_func($callBacks[$_POST['section']]);
        exit;
    }
}
Header("HTTP/1.0 404 Not Found");
echo "Not Found";
exit;

?>
