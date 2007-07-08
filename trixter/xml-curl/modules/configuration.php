<?php  //  -*- mode:c; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil;  -*-
function configuration()
{
    global $_SERVER;
    global $_POST;

    $smarty = new Smarty();
    $template = $_POST['key_value'].".tpl";

    // only stuff that we dynamically set gets done here, if its not here the file is sent as is
    switch($_POST['key_value']) {
    case "conference.conf":
        // simulate data from a database, perhaps a table with 3 columns did,value,args
        $params['pin'] = '1234';
        $params['anounce-count'] = '2';
        $params['max-mmebers'] = '15';
        $params['energy-level'] = '300';
        $params['member-flags'] = 'waste';

        $digits['0'] = 'event';
        $digits['1'] = 'event';
        $digits['2'] = 'event';
        $digits['3'] = 'event';
        $digits['4'] = 'event';
        $digits['5'] = 'event';
        $digits['6'] = 'event';
        $digits['7'] = 'event';
        $digits['8'] = 'event';
        $digits['9'] = 'event';
        $digits['#'] = 'event';
        $digits['*'] = 'event';

        $smarty->assign('CONFNAME',$_POST['conf_name']);
        $smarty->assign('digits',$digits);
        $smarty->assign('params',$params);
    }

    if($smarty->template_exists($_SERVER['DOCUMENT_ROOT'].dirname($_SERVER['PHP_SELF'])."/templates/".$_SERVER['REMOTE_USER']."/configuration/".$template)) {
        error_log("Issuing configuration $_POST[key_value]");
        $smarty->display($_SERVER['DOCUMENT_ROOT'].dirname($_SERVER['PHP_SELF'])."/templates/".$_SERVER['REMOTE_USER']."/configuration/".$template);
    } else {
        error_log("Unable to locate configuration file " . $_POST['key_value'] . " for " . $_SERVER['REMOTE_USER']);
        error_log("template: " . $_SERVER['DOCUMENT_ROOT'].dirname($_SERVER['PHP_SELF'])."/templates/".$_SERVER['REMOTE_USER']."/configuration/" . $template);
        return FALSE;
    }
    return TRUE;
}

// register the callback
$callBacks['configuration']='configuration';

?>
