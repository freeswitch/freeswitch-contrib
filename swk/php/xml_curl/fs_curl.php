<?

require_once "FSDirectory.php";
require_once "FSConfig.php";


header ("content-type: text/xml");

foreach ($_REQUEST as $key => $value) {
	$$key = $value;
}


/* Uncomment and edit for debugging directory*/
/*
$section = "directory";
$tag_name= "domain";
$key_name ="name";
$key_value="192.168.1.140";
$user="1000";
*/

/* Uncomment and edit for debugging directory*/
/*
$section = "configuration";
$tag_name= "configuration";
$key_name ="name";
$key_value="ivr.conf";
*/
/* section=configuration&tag_name=configuration&key_name=name&key_value=ivr.conf&Event-Name=REQUEST_PARAMS */

if ($section == "configuration" && $tag_name == "configuration" && $key_name == "name" && $key_value == "ivr.conf" ) {
	
	$fsc = new FSConfig();	
		
	printf(" <?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>\n");
	printf("   <document type=\"freeswitch/xml\">\n");
	printf("      <section name=\"configuration\" description=\"Codename: Shipment IVR\">\n");
	printf("        <configuration name=\"ivr.conf\" description=\"IVR menus\">\n");
	printf("          <menus>\n");

	$db_menus = $fsc->getConfigMenus();
	foreach ($db_menus as $db_menu) {
		printf("           <menu name=\"%s\"\n".
			"                greet-long=\"%s\"\n".
			"                greet-short=\"%s\"\n".
			"                invalid-sound=\"%s\"\n".
			"                exit-sound=\"%s\"\n".
			"                timeout=\"%s\"\n".
			"                inter-digit-timeout=\"%s\"\n".
			"                max-failures=\"%s\"\n".
			"                max-timeouts=\"%s\"\n".
			"                digit-len=\"%s\">\n",
			$db_menu['name'], $db_menu['greet_long'], $db_menu['greet_short'], $db_menu['invalid_sound'], 
			$db_menu['exit_sound'], $db_menu['timeout'], $db_menu['inter_digit_timeout'], $db_menu['max_failures'], 
			$db_menu['max_timeouts'], $db_menu['digit_len']);

		$db_menuopts = $fsc->getConfigMenuOptions($db_menu['uid']);

		foreach ($db_menuopts as $db_menuopt) {
			printf("            <entry action=\"%s\" digits=\"%s\" param=\"%s\"/>\n", 
				$db_menuopt['action'], $db_menuopt['digits'], $db_menuopt['param']);
		}	

		printf("           </menu>\n");
	}
	printf("          </menus>\n");
	printf("        </configuration>\n");
	printf("      </section>\n");
	printf("   </document>\n");

} else if ($section == "directory" && $tag_name == "domain" && $key_name == "name" && $user != "" ) {

	$fsd = new FSDirectory();
	$db_domain = $fsd->getDirDomainbyName($key_value);
	$db_domain_settings = $fsd->getDirDomain($db_domain['uid']);
	$db_user = $fsd->getDirUsersByDomainUidByUsername($db_domain['uid'], $user);
	$db_user_settings = $fsd->getDirUser($db_user['uid']);
	$db_groups = $fsd->getDirGroupsByDomianUidByUserUid($db_domain['uid'], $db_user['uid']);
	
	printf(" <?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>\n");
	printf("   <document type=\"freeswitch/xml\">\n");
	printf("      <section name=\"directory\" description=\"Codename: Shipment Directory\">\n");
	printf("         <domain name=\"%s\">\n", $db_domain['name']);
	printf("	  <params>\n");
	foreach($db_domain_settings['params'] as $db_params) {
		printf("	   <param name=\"%s\" value=\"%s\"/>\n", $db_params['name'], $db_params['value']);
	}
	printf("	  </params>\n");
	printf("	  <variables>\n");
	foreach($db_domain_settings['variables'] as $db_variables) {
		printf("	   <variable name=\"%s\" value=\"%s\"/>\n", $db_variables['name'], $db_variables['value']);
	}
	printf("	  </variables>\n");
	printf("           <groups>\n");
	printf("             <group name=\"default\">\n");
	printf("               <users>\n");
	printf("		 <user id=\"%s\" mailbox=\"%s\">\n", $db_user['username'], $db_user['mailbox']);
	printf("		  <params>\n");
	foreach($db_user_settings['params'] as $db_params) {
		printf("		   <param name=\"%s\" value=\"%s\"/>\n", $db_params['name'], $db_params['value']);
	}
	printf("		  </params>\n");
	printf("		  <variables>\n");
	foreach($db_user_settings['variables'] as $db_variables) {
		printf("		   <variable name=\"%s\" value=\"%s\"/>\n", $db_variables['name'], $db_variables['value']);
	}
	printf("		  </variables>\n");
	printf("		 </user>\n");
	printf("               </users>\n");
	printf("             </group>\n");
	foreach($db_groups as $db_group){
		printf("             <group name=\"%s\">\n", $db_group['groupName']);
		printf("               <users>\n");
		printf("		 <user id=\"%s\" type=\"pointer\"/>\n", $user);
		printf("               </users>\n");
		printf("             </group>\n");
	}
	
	printf("           </groups>\n");
	printf("         </domain>\n");
	printf("      </section>\n");
	printf("   </document>\n");
	
} else {
	printf("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>\n<document type=\"freeswitch/xml\">\n<section name=\"result\">\n".
    		"<result status=\"not found\" />\n</section>\n</document>");
	die();
}
?>
