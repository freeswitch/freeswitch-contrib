<?
/* Simple Script to generate a Domain and a group of users */

if($argv[1] == "-h" || $argv[1] == "-help" || $argv[1] == "-?"){
	printf("%s <domain_name> <start_exten> <stop_exten> <sip_pass>\n", $argv[0]);
	printf("Example: %s domain.com 1000 1019 1234\n", $argv[0]);
	die();
}

$domain_name = $argv[1];
$start = $argv[2];
$stop  = $argv[3];
$sip_pass = $argv[4];

require_once("freeswitch.php");

$domains_params_insert = array(array('dial-string','{presence_id=\${dialed_user}@\${dialed_domain}}\${sofia_contact(\${dialed_user}@\${dialed_domain})}'));

$domains_vars_insert = array(	array('record_stereo','true'),
				array('default_gateway','\$\${default_provider}'),
				array('default_areacode','\$\${default_areacode}'),
				array('transfer_fallback_extension','operator'));

$user_default_vars  = array(	array("numbering_plan", "\$\${default_country}"),
      				array("default_areacode", "\$\${default_areacode}"),
      				array("default_gateway", "\$\${default_provider}"));

$user_params_insert = array(	array('password',$sip_pass),
				array('vm-password','%s'));

$user_vars_insert = array(	array('toll_allow','domestic,international,local'),
				array('accountcode','%s'),
				array('user_context','default'),
				array('effective_caller_id_name','Extension %s'),
				array('effective_caller_id_number','%s'),
				array('outbound_caller_id_name','\$\${outbound_caller_name}'),
				array('outbound_caller_id_number','\$\${outbound_caller_id}'),
				array('callgroup','techsupport'));


$fsc = new FreeSWITCH();

if ($fsc->addDirDomain($domain_name) < 1){
	die("add domain failed\n");
}

$db_domain = $fsc->getDirDomainbyName($domain_name);

foreach($domains_params_insert as $param){
	if ($fsc->addDirDomainParam($db_domain['uid'], $param[0], $param[1]) < 1){
		die("Add Domain Param Failed\n");
	}
} 

foreach($domains_vars_insert as $vars){
	if ($fsc->addDirDomainVar($db_domain['uid'], $vars[0], $vars[1]) < 1 ){
		die("Add Domain Var Failed\n");
	}
} 

if ($fsc->addDirDomainUser($db_domain['uid'], "default", "", "", 1) < 1) {
	die("add default user failed\n");
}

$db_user = $fsc->getDirUsersByDomainUidByUsername($db_domain['uid'], "default");

foreach($user_default_vars as $vars){
	if (preg_match('/%s/', $vars[1])) {
		$temp = $vars[1];
		$vars[1] = sprintf($temp, "default");
	}
	if ($fsc->addDirDomainUserVar($db_user['uid'], $vars[0], $vars[1]) < 1 ) {
		die("Add Default User Var Failed\n");
	}
} 

for ($i = $start; $i <= $stop; $i++) {

	if ($fsc->addDirDomainUser($db_domain['uid'], $i, $i, "", 1) < 1) {
		die("add user failed\n");
	}
	
	$db_user = $fsc->getDirUsersByDomainUidByUsername($db_domain['uid'], $i);
	
	foreach($user_params_insert as $param){
		if (preg_match('/%s/', $param[1])) {
			$temp = $param[1];
			$param[1] = sprintf($temp, $i);
		}
		if ($fsc->addDirDomainUserParam($db_user['uid'], $param[0], $param[1]) < 1){
			die("Add User Param Failed\n");
		}
	} 

	foreach($user_vars_insert as $vars){
		if (preg_match('/%s/', $vars[1])) {
			$temp = $vars[1];
			$vars[1] = sprintf($temp, $i);
		}
		if ($fsc->addDirDomainUserVar($db_user['uid'], $vars[0], $vars[1]) < 1 ) {
			die("Add User Var Failed\n");
		}
	} 
}

?>
