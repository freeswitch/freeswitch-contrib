<?php  //  -*- mode:c; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil;  -*-
function phrases()
{
    global $_SERVER;
    global $_POST;


    // create a new smarty object
	$smarty = new Smarty();
	$template = 'lang_'. $_POST['lang'] .'.tpl';

	$smarty->display($_SERVER['DOCUMENT_ROOT'].dirname($_SERVER['PHP_SELF'])."/templates/".$_SERVER['REMOTE_USER']."/phrases/".$template);
    return TRUE;
}


// register the callback
$callBacks['phrases']='phrases';

?>
