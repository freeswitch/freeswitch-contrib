<?php  //  -*- mode:c; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil;  -*-

/* generally we get the following information

  [section] => directory
  [tag_name] => domain
  [key_name] => name
  [key_value] => 192.168.153.128
  [user] => testacct
  [domain] => 192.168.153.128
  [ip] => 192.168.153.1

*/

function directory()
{
    global $_SERVER;
    global $_POST;


    // create a new smarty object
	$smarty = new Smarty();
	$template = $_POST['domain'] .'.tpl';

    // simulate a database query
    $smarty->assign('USER','testacct');
    $smarty->assign('PASSWORD','demopw');
    $smarty->assign('DOMAIN',$_POST['domain']);


	$smarty->display($_SERVER['DOCUMENT_ROOT'].dirname($_SERVER['PHP_SELF'])."/templates/".$_SERVER['REMOTE_USER']."/directory/".$template);
	error_log($smarty->fetch($_SERVER['DOCUMENT_ROOT'].dirname($_SERVER['PHP_SELF'])."/templates/".$_SERVER['REMOTE_USER']."/directory/".$template));
    error_log(print_r($_POST,true));
    return TRUE;
}


// register the callback
$callBacks['directory']='directory';

?>
