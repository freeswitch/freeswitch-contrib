<?php //  -*- mode:c; tab-width:4; c-basic-offset:4; c-indent-level:4; indent-tabs-mode:nil;  -*-
/*
 * example for http://www.cluecon.com 2007 
 *
 * this program illustrates some of the XML-RPC functionality
 * 
 * because this is an example its not the best design.  For example, it would be better
 * to have a cron job or something store this in a DB instead of hammering your FS box 
 * each time you want to see what calls are in.  It would also make it easier to hangup,
 * kick, etc the calls in progress.  DB usage is beyond the scope of this example however.
 *
 * I did try to get people thinking about some of the problems that might be encountered 
 * in a 'real app' such as multilanguage support, seperation of html from content (so non-devs
 * can more easily update the look and feel as well as just better design), support for
 * multiple FS boxes, etc.
 * 
 * specifically the multilanguage approach used here is not good at all.  Smarty has some 
 * multilanguage stuff that can be better than this, and there are a lot of other packages
 * the idea was just to get people to think about it :)
 */
ini_set("include_path",".:/usr/local/lib/php:/usr/share/php:./Smarty-2.6.18/libs");
include_once("fsxmlrpc.inc");
include_once('Smarty.class.php');



$hosts = array (
                0 => array ( "hostname" => "switch1.my.tld",
                             "port"     => 8080,
                             "username" => "username",
                             "password" => "password"
                             ),
                1 => array ( "hostname" => "switch2.my.tld",
                             "port"     => 8080,
                             "username" => "username",
                             "password" => "password"
                             ),
                );



$smarty    = new Smarty;
$chanArray = Array();
$confArray = Array();

function connect($host)
{
    $client=new fsxmlrpc("/RPC2",$host['hostname'] , $host['port']);
    $client->setCredentials($host['username'],$host['password'],NULL);
    return $client;
}


function findGateway($host)
{
    global $hosts;
    foreach ($hosts as $key => $value) {
        if($hosts[$key]['hostname'] == $host) {
            return $hosts[$key];
        }
    }
    return false;
}


$language='default';
if(isset($_GET['hl']) && $_GET['hl']!='') {
    // security warning, I dont check for ../../ this is just an example afterall
    if(file_exists($_SERVER['DOCUMENT_ROOT'].dirname($_SERVER['PHP_SELF'])."/themes/" . $_GET['hl'] . "/index.tpl")) {
        $language=$_GET['hl'];
    }
}

include_once($_SERVER['DOCUMENT_ROOT'].dirname($_SERVER['PHP_SELF'])."/themes/" . $language . "/errors.inc");


if(isset($_POST['action']) && $_POST['action']!='') {
	switch($_POST['action']) {
    case "kick":
        if(isset($_POST['memid']) && isset($_POST['conf'])) {
            foreach($_POST['memid'] as $singleVar) {
                $client=connect(findGateway($_POST['gateway']));
                if($client->conferenceKick($_POST['conf'],$singleVar)) {
                    $errors[]=$ERRMSG[KICK_SUCCESS] . "$_POST[conf]:$singleVar";
                } else {
                    $errors[]=$ERRMSG[KICK_FAIL] . "$_POST[conf]:$singleVar";
                }
            }
        }
        break;
    case "mute":
        if(isset($_POST['memid']) && isset($_POST['conf'])) {
            foreach($_POST['memid'] as $singleVar) {
                $client=connect(findGateway($_POST['gateway']));
                if($client->conferenceMute($_POST['conf'],$singleVar)) {
                    $errors[]=$ERRMSG[MUTE_SUCCESS] . "$_POST[conf]:$singleVar";
                } else {
                    $errors[]=$ERRMSG[MUTE_FAIL] . "$_POST[conf]:$singleVar";
                }
            }
        }
        break;
    case "unmute":
        if(isset($_POST['memid']) && isset($_POST['conf'])) {
            foreach($_POST['memid'] as $singleVar) {
                $client=connect(findGateway($_POST['gateway']));
                if($client->conferenceUnmute($_POST['conf'],$singleVar)) {
                    $errors[]=$ERRMSG[UNMUTE_SUCCESS] . "$_POST[conf]:$singleVar";
                } else {
                    $errors[]=$ERRMSG[UNMUTE_FAIL] . "$_POST[conf]:$singleVar";
                }
            }
        }
        break;
    case "deaf":
        if(isset($_POST['memid']) && isset($_POST['conf'])) {
            foreach($_POST['memid'] as $singleVar) {
                $client=connect(findGateway($_POST['gateway']));
                if($client->conferenceDeaf($_POST['conf'],$singleVar)) {
                    $errors[]=$ERRMSG[DEAF_SUCCESS] . "$_POST[conf]:$singleVar";
                } else {
                    $errors[]=$ERRMSG[DEAF_FAIL] . "$_POST[conf]:$singleVar";
                }
            }
        }
        break;
    case "undeaf":
        if(isset($_POST['memid']) && isset($_POST['conf'])) {
            foreach($_POST['memid'] as $singleVar) {
                $client=connect(findGateway($_POST['gateway']));
                if($client->conferenceUndeaf($_POST['conf'],$singleVar)) {
                    $errors[]=$ERRMSG[UNDEAF_SUCCESS] . "$_POST[conf]:$singleVar";
                } else {
                    $errors[]=$ERRMSG[UNDEAF_FAIL] . "$_POST[conf]:$singleVar";
                }
            }
        }
        break;
	}
}



foreach ($hosts as $i => $value) {
    $client=connect($hosts[$i]);
    //    $client->setDebug(2);
    
    $chanArray = array_merge($chanArray,$client->getChannels());
    $ca = $client->conferenceList();

    // we need to track the gateway so we can perform operations on the caller
    foreach ($ca as $j => $value) {
        $ca[$j][gateway] = $hosts[$i]['hostname'];
    }
    $confArray = array_merge($confArray,$ca);
}

// echo "<pre>".print_r($confArray,true)."</pre>";

$smarty->assign('errors',$errors);
$smarty->assign('channels',$chanArray);
$smarty->assign('conferences',$confArray);


$smarty->template_dir=$_SERVER['DOCUMENT_ROOT'].dirname($_SERVER['PHP_SELF'])."/themes/" . $language . "/";
$smarty->display("index.tpl");
?>
