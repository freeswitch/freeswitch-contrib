<?

require_once "FSDirectory.php";


header ("content-type: text/xml");

foreach ($_REQUEST as $key => $value) {
	$$key = $value;
}

$fsd = new FSDirectory();

/* Uncomment and edit for debugging */
/*
$section = "directory";
$tag_name= "domain";
$key_name ="name";
$key_value="192.168.1.140";
$user="1000";
*/
if ($section == "directory" && $tag_name == "domain" && $key_name == "name") {
	$db_domain = $fsd->getDirDomainbyName($key_value);
} else {
	printf("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>\n<document type=\"freeswitch/xml\">\n<section name=\"result\">\n".
    		"<result status=\"not found\" />\n</section>\n</document>");
	die();
}

$db_user = $fsd->getDirUsersByDomainUidByUsername($db_domain['uid'], $user);

$db_user_settings = $fsd->getDirUser($db_user['uid']);

$db_groups = $fsd->getDirGroups($db_domain['uid']);

printf(" <?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>\n");
printf("   <document type=\"freeswitch/xml\">\n");
printf("      <section name=\"directory\" description=\"Codename: Shipment Directory\">\n");
printf("         <domain name=\"%s\">\n", $db_domain['name']);
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
/*  This is Broken need to talk to someone about this part... maybe I should load this at boot time instead 
foreach($db_groups as $db_group){
	$db_members = $fsd->getDirGroup($db_group['uid']);
	printf("             <group name=\"%s\">\n", $db_group['name']);
	printf("               <users>\n");
	foreach($db_members['members'] as $db_member){
		printf("		 <user id=\"%s\" type=\"pointer\"/>\n", $db_member['usersUsername']);
	}
	printf("               </users>\n");
	printf("             </group>\n");
}
*/
printf("           </groups>\n");
printf("         </domain>\n");
printf("      </section>\n");
printf("   </document>\n");
?>
