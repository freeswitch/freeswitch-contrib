<?php  //  -*- mode:c; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil;  -*-
Header("Content-type: text/plain");

ini_set("include_path",".:/usr/local/lib/php:/usr/share/php:./Smarty-2.6.18/libs");
include_once('Smarty.class.php');


if (isset($_POST['section']) && $_POST['section']=="dialplan") {
	// create a new smarty object
	$smarty = new Smarty();
	$template = 'fail.tpl';


	// see if they authenticated via .htaccess
	if(isset($_SERVER['REMOTE_USER']) && $_SERVER['REMOTE_USER']!="") {
        // we might check a database for  the route, but in this example we hardcode it
        $template = 'frompstn.tpl'; // one can exist for each profile or channel type or ...

        $ACTION['javascript']="somethingcool.js";
        $ACTION['bridge']="sofia/external/".$_POST['destination_number']."@myprovider.tld";
        $smarty->assign('ACTION',$ACTION);

    } // end - they were logged in
	$smarty->display($_SERVER['DOCUMENT_ROOT']."/templates/".$_SERVER['REMOTE_USER']."/".$template);
} else if (isset($_POST['section']) && $_POST['section']=="configuration") {
		$smarty = new Smarty();
		$template = $_POST['key_value'].".tpl";
		switch($_POST['key_value']) {
        case "conference.conf":
            // simulate data from a database, perhaps a table with 3 columns did,value,args
            $digits['pin'] = '1234';
            $digits['anounce-count'] = '2';
            $digits['max-mmebers'] = '15';
            $digits['energy-level'] = '300';
            $params['0'] = 'event';
            $params['1'] = 'event';
            $params['2'] = 'event';
            $params['3'] = 'event';
            $params['4'] = 'event';
            $params['5'] = 'event';
            $params['6'] = 'event';
            $params['7'] = 'event';
            $params['8'] = 'event';
            $params['9'] = 'event';
            $params['#'] = 'event';
            $params['*'] = 'event';


            $smarty->assign('CONFNAME',$_POST['conf_name']);
            $smarty->assign('digits',$digits);
            $smarty->assign('params',$params);
		}
		if($smarty->template_exists($_SERVER['DOCUMENT_ROOT']."/templates/".$_SERVER['REMOTE_USER']."/".$template)) {
			$smarty->display($_SERVER['DOCUMENT_ROOT']."/templates/".$_SERVER['REMOTE_USER']."/".$template);
		} else {
            //			//logCallData();
			error_log("Unable to locate template $template");
			$smarty->assign('CONFNAME',$_POST['key_value']);
			$smarty->display($_SERVER['DOCUMENT_ROOT']."/templates/".$_SERVER['REMOTE_USER']."/default.conf.tpl");
		}
} else if (isset($_POST['section']) && $_POST['section']=="directory") {
    // we dont do anything but you get the idea
}

?>
