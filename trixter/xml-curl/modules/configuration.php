<?php  //  -*- mode:c; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil;  -*-
function configuration()
{
    global $_SERVER;
    global $_POST;

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
    if($smarty->template_exists($_SERVER['DOCUMENT_ROOT'].dirname($_SERVER['PHP_SELF'])."/templates/".$_SERVER['REMOTE_USER']."/".$template)) {
        $smarty->display($_SERVER['DOCUMENT_ROOT'].dirname($_SERVER['PHP_SELF'])."/templates/".$_SERVER['REMOTE_USER']."/".$template);
    } else {
        //			//logCallData();
        error_log("Unable to locate template $template");
        $smarty->assign('CONFNAME',$_POST['key_value']);
        $smarty->display($_SERVER['DOCUMENT_ROOT'].dirname($_SERVER['PHP_SELF'])."/templates/".$_SERVER['REMOTE_USER']."/default.conf.tpl");
    }
}

// register the callback
$callBacks['configuration']='configuration';

?>
