<?php  //  -*- mode:c; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil;  -*-
function dialplan()
{
    global $_SERVER;
    global $_POST;


    // create a new smarty object
	$smarty = new Smarty();
	$template = 'fail.tpl';


	// see if they authenticated via .htaccess
	if(isset($_SERVER['REMOTE_USER']) && $_SERVER['REMOTE_USER']!="") {
        // we might check a database for  the route, but in this example we hardcode it
        $template = 'frompstn.tpl'; // one can exist for each profile or channel type or ...

        // simulate a database query for the dialplan
        $ACTION[]=Array('answer','');
        $ACTION[]=Array('phrase','spell,abcdef');
        $ACTION[]=Array('javascript','somethingcool.js');
        $ACTION[]=Array('bridge','sofia/external/$_POST[destination_number]@myprovider.tld');
        $smarty->assign('ACTION',$ACTION);

    } // end - they were logged in
	$smarty->display($_SERVER['DOCUMENT_ROOT'].dirname($_SERVER['PHP_SELF'])."/templates/".$_SERVER['REMOTE_USER']."/dialplan/".$template);

    return TRUE;
}


// register the callback
$callBacks['dialplan']='dialplan';

?>
