-- MySQL dump 10.11
--
-- Host: localhost    Database: freeswitch
-- ------------------------------------------------------
-- Server version	5.0.45-community-nt

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;

--
-- Table structure for table `conference_advertise`
--

DROP TABLE IF EXISTS `conference_advertise`;
CREATE TABLE `conference_advertise` (
  `id` int(10) unsigned NOT NULL auto_increment,
  `room` varchar(64) NOT NULL,
  `status` varchar(128) NOT NULL,
  PRIMARY KEY  (`id`),
  UNIQUE KEY `unique_room` (`room`)
) ENGINE=InnoDB AUTO_INCREMENT=8 DEFAULT CHARSET=latin1;

--
-- Dumping data for table `conference_advertise`
--

LOCK TABLES `conference_advertise` WRITE;
/*!40000 ALTER TABLE `conference_advertise` DISABLE KEYS */;
INSERT INTO `conference_advertise` VALUES (1,'3000@$${domain}','Freeswitch Conference'),(2,'3001@$${domain}','FreeSWITCH Conference 2'),(3,'3002@$${domain}','FreeSWITCH Conference 3');
/*!40000 ALTER TABLE `conference_advertise` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `conference_controls`
--

DROP TABLE IF EXISTS `conference_controls`;
CREATE TABLE `conference_controls` (
  `id` int(10) unsigned NOT NULL auto_increment,
  `conf_group` varchar(64) NOT NULL,
  `action` varchar(64) NOT NULL,
  `digits` varchar(16) NOT NULL,
  PRIMARY KEY  (`id`),
  UNIQUE KEY `unique_group_action` USING BTREE (`conf_group`,`action`)
) ENGINE=InnoDB AUTO_INCREMENT=25 DEFAULT CHARSET=latin1;

--
-- Dumping data for table `conference_controls`
--

LOCK TABLES `conference_controls` WRITE;
/*!40000 ALTER TABLE `conference_controls` DISABLE KEYS */;
INSERT INTO `conference_controls` VALUES (1,'default','mute','0'),(2,'default','deaf_mute','*'),(3,'default','energy up','9'),(4,'default','energy equ','8'),(5,'default','energy dn','7'),(6,'default','vol talk up','3'),(7,'default','vol talk dn','1'),(8,'default','vol talk zero','2'),(9,'default','vol listen up','6'),(10,'default','vol listen dn','4'),(11,'default','vol listen zero','5'),(12,'default','hangup','#');
/*!40000 ALTER TABLE `conference_controls` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `conference_profiles`
--

DROP TABLE IF EXISTS `conference_profiles`;
CREATE TABLE `conference_profiles` (
  `id` int(10) unsigned NOT NULL auto_increment,
  `profile_name` varchar(64) NOT NULL,
  `param_name` varchar(64) NOT NULL,
  `param_value` varchar(64) NOT NULL,
  PRIMARY KEY  (`id`),
  KEY `unique_profile_param` (`profile_name`,`param_name`)
) ENGINE=InnoDB AUTO_INCREMENT=13 DEFAULT CHARSET=latin1;

--
-- Dumping data for table `conference_profiles`
--

LOCK TABLES `conference_profiles` WRITE;
/*!40000 ALTER TABLE `conference_profiles` DISABLE KEYS */;
INSERT INTO `conference_profiles` VALUES (1,'default','domain','$${domain}'),(2,'default','rate','8000'),(3,'default','interval','20'),(4,'default','energy-level','300'),(5,'default','moh-sound','$${moh_uri}'),(6,'default','caller-id-name','$${outbound_caller_name}'),(7,'default','caller-id-number','$${outbound_caller_number}');
/*!40000 ALTER TABLE `conference_profiles` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `dialplan`
--

DROP TABLE IF EXISTS `dialplan`;
CREATE TABLE `dialplan` (
  `id` int(11) NOT NULL auto_increment,
  `context` varchar(64) NOT NULL,
  `extension` varchar(64) NOT NULL,
  `condition_field` varchar(64) NOT NULL,
  `condition_expression` varchar(64) NOT NULL,
  `application_name` varchar(64) NOT NULL,
  `application_data` text,
  `weight` int(11) NOT NULL,
  `type` varchar(16) NOT NULL default 'action',
  `ext_continue` smallint(1) NOT NULL default '0',
  `global_weight` int(11) NOT NULL default '10000',
  `cond_break` varchar(8) default NULL,
  PRIMARY KEY  (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=385 DEFAULT CHARSET=utf8;

--
-- Dumping data for table `dialplan`
--

LOCK TABLES `dialplan` WRITE;
/*!40000 ALTER TABLE `dialplan` DISABLE KEYS */;
INSERT INTO `dialplan` VALUES (273,'US-Numbering-Plan','US_International','destination_number','^011(\\d+)$','enum','1$1',3,'action',0,200,''),(272,'US-Numbering-Plan','US_International','destination_number','^011(\\d+)$','set','hangup_after_bridge=true',2,'action',0,200,''),(271,'US-Numbering-Plan','US_International','destination_number','^011(\\d+)$','set','continue_on_fail=true',1,'action',0,200,''),(383,'default','hold_music','destination_number','^9999$','playback','$${moh_uri}',2,'action',0,1900,''),(384,'default','enum','destination_number','^(.*)$','transfer','$1 enum',1,'action',0,2000,''),(382,'default','hold_music','destination_number','^9999$','answer','',1,'action',0,1900,''),(378,'default','ivr_demo','destination_number','5000','ivr','demo',1,'action',0,1600,''),(379,'default','park','destination_number','^(590[0-9])$','fifo','$1@$${domain} in undef $${moh_uri}',1,'action',0,1700,''),(380,'default','unpark','destination_number','^\\*{0,2}park\\+(590[0-9])$','answer','',1,'action',0,1800,''),(381,'default','unpark','destination_number','^\\*{0,2}park\\+(590[0-9])$','fifo','$1@$${domain} out nowait',2,'action',0,1800,''),(375,'default','sip_uri','destination_number','^sip:(.*)$','bridge','sofia/${use_profile}/$1',1,'action',0,1300,''),(376,'default','conferences','destination_number','^(3\\d{3})$','conference','$1@default',1,'action',0,1400,''),(377,'default','freeswitch_public_conf_via_sip','destination_number','^9(888|1616)$','bridge','sofia/${use_profile}/$1@conference.freeswitch.org',1,'action',0,1500,''),(372,'default','Local_Extension','destination_number','^${caller_id_number}$','answer','',13,'anti-action',0,1200,''),(373,'default','Local_Extension','destination_number','^${caller_id_number}$','sleep','1000',14,'anti-action',0,1200,''),(374,'default','Local_Extension','destination_number','^${caller_id_number}$','voicemail','default $${domain} ${dialed_ext}',15,'anti-action',0,1200,''),(371,'default','Local_Extension','destination_number','^${caller_id_number}$','bridge','{left_hanging_extension=5900,transfer_fallback_extension=${dialed_ext}}USER/${dialed_ext}@$${domain}',12,'anti-action',0,1200,''),(367,'default','Local_Extension','destination_number','^${caller_id_number}$','set','left_hanging_extension=5900',8,'anti-action',0,1200,''),(368,'default','Local_Extension','destination_number','^${caller_id_number}$','set','continue_on_fail=true',9,'anti-action',0,1200,''),(369,'default','Local_Extension','destination_number','^${caller_id_number}$','db','insert/call_return/${dialed_ext}/${caller_id_number}',10,'anti-action',0,1200,''),(370,'default','Local_Extension','destination_number','^${caller_id_number}$','db','insert/last_dial_ext/${dialed_ext}/${uuid}',11,'anti-action',0,1200,''),(364,'default','Local_Extension','destination_number','^${caller_id_number}$','ring_ready','',5,'anti-action',0,1200,''),(365,'default','Local_Extension','destination_number','^${caller_id_number}$','set','call_timeout=130',6,'anti-action',0,1200,''),(366,'default','Local_Extension','destination_number','^${caller_id_number}$','set','hangup_after_bridge=true',7,'anti-action',0,1200,''),(361,'default','Local_Extension','destination_number','^${caller_id_number}$','answer','',2,'action',0,1200,''),(362,'default','Local_Extension','destination_number','^${caller_id_number}$','sleep','1000',3,'action',0,1200,''),(363,'default','Local_Extension','destination_number','^${caller_id_number}$','voicemail','check default $${domain} ${dialed_ext}',4,'action',0,1200,''),(359,'default','Local_Extension','destination_number','^(10[01][0-9])$','set','dialed_ext=$1',1,'action',0,1200,'on-true'),(360,'default','Local_Extension','destination_number','^${caller_id_number}$','set','voicemail_authorized=${sip_authorized}',1,'action',0,1200,''),(357,'default','call-group-order','destination_number','^83(\\d{2})$','set','call_timeout=10',1,'action',0,1100,''),(358,'default','call-group-order','destination_number','^83(\\d{2})$','bridge','${group(call:$1:order)}',2,'action',0,1100,''),(356,'default','call-group-simo','destination_number','^82(\\d{2})$','bridge','${group(call:$1)}',1,'action',0,1000,''),(355,'default','add-group','destination_number','^81(\\d{2})$','gentones','%(1000, 0, 640)',3,'action',0,900,''),(354,'default','add-group','destination_number','^81(\\d{2})$','group','insert:$1:${sofia_contact(${sip_from_user}@${domain})}',2,'action',0,900,''),(350,'default','del-group','destination_number','^80(\\d{2})$','answer','',1,'action',0,800,''),(351,'default','del-group','destination_number','^80(\\d{2})$','group','delete:$1:${sofia_contact(${sip_from_user}@${domain})}',2,'action',0,800,''),(352,'default','del-group','destination_number','^80(\\d{2})$','gentones','%(1000, 0, 320)',3,'action',0,800,''),(353,'default','add-group','destination_number','^81(\\d{2})$','answer','',1,'action',0,900,''),(349,'default','call_return','destination_number','^\\*69$|^869$','transfer','${db(select/call_return/${caller_id_number})}',1,'action',0,700,''),(348,'default','eavesdrop','destination_number','^88(.*)$|^\\*0(.*)$','eavesdrop','${db(select/spymap/$1)}',2,'action',0,600,''),(347,'default','eavesdrop','destination_number','^88(.*)$|^\\*0(.*)$','answer','',1,'action',0,600,''),(344,'default','global','','','db','insert/spymap/${caller_id_number}/${uuid}',2,'action',1,500,''),(345,'default','global','','','db','insert/last_dial/${caller_id_number}/${destination_number}',3,'action',1,500,''),(346,'default','global','','','db','insert/last_dial/global/${uuid}',4,'action',1,500,''),(340,'default','global','${network_addr}','','set','use_profile=${cond(${is_lan_addr($${local_ip_v4})} == yes ? nat : default)}',1,'action',1,500,''),(342,'default','global','${numbering_plan}','','set_user','default@${domain}',1,'action',1,500,''),(343,'default','global','','','info','',1,'action',1,500,''),(341,'default','global','${network_addr}','','set','use_profile=${cond(${is_lan_addr(${network_addr})} == yes ? nat : default)}',2,'anti-action',1,500,''),(338,'default','intercept-ext','destination_number','^\\*\\*(\\d+)$','sleep','2000',3,'action',0,300,''),(339,'default','redial','destination_number','^870$','transfer','${db(select/last_dial/${caller_id_number})}',1,'action',0,400,''),(337,'default','intercept-ext','destination_number','^\\*\\*(\\d+)$','intercept','${db(select/last_dial_ext/$1)}',2,'action',0,300,''),(335,'default','intercept','destination_number','^886$','sleep','2000',3,'action',0,200,''),(336,'default','intercept-ext','destination_number','^\\*\\*(\\d+)$','answer','',1,'action',0,300,''),(333,'default','intercept','destination_number','^886$','answer','',1,'action',0,200,''),(334,'default','intercept','destination_number','^886$','intercept','${db(select/last_dial/global)}',2,'action',0,200,''),(113,'public','public_extensions','destination_number','^(10[01][0-9])$','transfer','$1 XML default',1,'action',0,200,''),(114,'public','public_did','destination_number','^(5551212)$','transfer','$1 XML default',1,'action',0,300,''),(274,'US-Numbering-Plan','US_International','destination_number','^011(\\d+)$','bridge','${enum_auto_route}',4,'action',0,200,''),(275,'US-Numbering-Plan','US_International','destination_number','^011(\\d+)$','bridge','sofia/gateway/${default_gateway}/011$1',5,'action',0,200,''),(276,'US-Numbering-Plan','US_LD','destination_number','^1?([2-9]\\d{2}[2-9]\\d{6})$','set','continue_on_fail=true',1,'action',0,300,''),(277,'US-Numbering-Plan','US_LD','destination_number','^1?([2-9]\\d{2}[2-9]\\d{6})$','set','hangup_after_bridge=true',2,'action',0,300,''),(278,'US-Numbering-Plan','US_LD','destination_number','^1?([2-9]\\d{2}[2-9]\\d{6})$','enum','1$1',3,'action',0,300,''),(279,'US-Numbering-Plan','US_LD','destination_number','^1?([2-9]\\d{2}[2-9]\\d{6})$','bridge','${enum_auto_route}',4,'action',0,300,''),(280,'US-Numbering-Plan','US_LD','destination_number','^1?([2-9]\\d{2}[2-9]\\d{6})$','bridge','sofia/gateway/${default_gateway}/1$1',5,'action',0,300,''),(281,'US-Numbering-Plan','US_Local','${default_area_code}','\\d{3}','say','you must dial the area code to call this destination',1,'anti-action',0,400,'on-true'),(282,'US-Numbering-Plan','US_Local','destination_number','^([2-9]\\d{6})$','set','continue_on_fail=true',1,'action',0,400,''),(283,'US-Numbering-Plan','US_Local','destination_number','^([2-9]\\d{6})$','set','hangup_after_bridge=true',2,'action',0,400,''),(284,'US-Numbering-Plan','US_Local','destination_number','^([2-9]\\d{6})$','enum','1$1',3,'action',0,400,''),(285,'US-Numbering-Plan','US_Local','destination_number','^([2-9]\\d{6})$','bridge','${enum_auto_route}',4,'action',0,400,''),(286,'US-Numbering-Plan','US_Local','destination_number','^([2-9]\\d{6})$','bridge','sofia/gateway/${default_gateway}/1${default_area_code}$1',5,'action',0,400,''),(287,'US-Numbering-Plan','FCC_Services','^([4689]11)$','','bridge','sofia/gateway/${default_gateway}/$1',1,'action',0,500,'');
/*!40000 ALTER TABLE `dialplan` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `dingaling_profile_params`
--

DROP TABLE IF EXISTS `dingaling_profile_params`;
CREATE TABLE `dingaling_profile_params` (
  `id` int(10) unsigned NOT NULL auto_increment,
  `dingaling_id` int(10) unsigned NOT NULL,
  `param_name` varchar(64) NOT NULL,
  `param_value` varchar(64) NOT NULL,
  PRIMARY KEY  (`id`),
  UNIQUE KEY `unique_type_name` (`dingaling_id`,`param_name`),
  CONSTRAINT `dingaling_profile` FOREIGN KEY (`dingaling_id`) REFERENCES `dingaling_profiles` (`id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB AUTO_INCREMENT=5 DEFAULT CHARSET=latin1;

--
-- Dumping data for table `dingaling_profile_params`
--

LOCK TABLES `dingaling_profile_params` WRITE;
/*!40000 ALTER TABLE `dingaling_profile_params` DISABLE KEYS */;
INSERT INTO `dingaling_profile_params` VALUES (1,1,'password','secret'),(2,1,'dialplan','XML,enum'),(3,1,'server','example.org'),(4,1,'name','fs.example.org');
/*!40000 ALTER TABLE `dingaling_profile_params` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `dingaling_profiles`
--

DROP TABLE IF EXISTS `dingaling_profiles`;
CREATE TABLE `dingaling_profiles` (
  `id` int(10) unsigned NOT NULL auto_increment,
  `profile_name` varchar(64) NOT NULL,
  `type` varchar(64) NOT NULL,
  PRIMARY KEY  (`id`),
  UNIQUE KEY `unique_name` (`profile_name`)
) ENGINE=InnoDB AUTO_INCREMENT=3 DEFAULT CHARSET=latin1;

--
-- Dumping data for table `dingaling_profiles`
--

LOCK TABLES `dingaling_profiles` WRITE;
/*!40000 ALTER TABLE `dingaling_profiles` DISABLE KEYS */;
INSERT INTO `dingaling_profiles` VALUES (1,'fs.intralanman.servehttp.com','component');
/*!40000 ALTER TABLE `dingaling_profiles` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `dingaling_settings`
--

DROP TABLE IF EXISTS `dingaling_settings`;
CREATE TABLE `dingaling_settings` (
  `id` int(10) unsigned NOT NULL auto_increment,
  `param_name` varchar(64) NOT NULL,
  `param_value` varchar(64) NOT NULL,
  PRIMARY KEY  (`id`),
  UNIQUE KEY `unique_param` (`param_name`)
) ENGINE=InnoDB AUTO_INCREMENT=3 DEFAULT CHARSET=latin1;

--
-- Dumping data for table `dingaling_settings`
--

LOCK TABLES `dingaling_settings` WRITE;
/*!40000 ALTER TABLE `dingaling_settings` DISABLE KEYS */;
INSERT INTO `dingaling_settings` VALUES (1,'debug','0'),(2,'codec-prefs','$${global_codec_prefs}');
/*!40000 ALTER TABLE `dingaling_settings` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `directory`
--

DROP TABLE IF EXISTS `directory`;
CREATE TABLE `directory` (
  `id` int(11) NOT NULL auto_increment,
  `username` varchar(255) NOT NULL,
  `mailbox` varchar(255) NOT NULL,
  `domain` varchar(255) NOT NULL,
  PRIMARY KEY  (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=12 DEFAULT CHARSET=utf8;

--
-- Dumping data for table `directory`
--

LOCK TABLES `directory` WRITE;
/*!40000 ALTER TABLE `directory` DISABLE KEYS */;
INSERT INTO `directory` VALUES (1,'1000','','example.com'),(2,'1001','','example.org'),(3,'1002','','example.net'),(5,'1003','','example.info'),(6,'1004','','example.com'),(7,'1005','','example.org'),(8,'1006','','example.net'),(9,'1007','','example.info'),(10,'1008','','$${domain}'),(11,'1009','','$${local_ip_v4}'),(12,'tester','1000','$${domain}');
/*!40000 ALTER TABLE `directory` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `directory_domains`
--

DROP TABLE IF EXISTS `directory_domains`;
CREATE TABLE `directory_domains` (
  `id` int(10) unsigned NOT NULL auto_increment,
  `domain_name` varchar(128) NOT NULL,
  PRIMARY KEY  (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=3 DEFAULT CHARSET=latin1;

--
-- Dumping data for table `directory_domains`
--

LOCK TABLES `directory_domains` WRITE;
/*!40000 ALTER TABLE `directory_domains` DISABLE KEYS */;
INSERT INTO `directory_domains` VALUES (1,'freeswitch.org'),(2,'sofaswitch.org');
/*!40000 ALTER TABLE `directory_domains` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `directory_gateway_params`
--

DROP TABLE IF EXISTS `directory_gateway_params`;
CREATE TABLE `directory_gateway_params` (
  `id` int(10) unsigned NOT NULL auto_increment,
  `d_gw_id` int(10) unsigned NOT NULL,
  `param_name` varchar(64) NOT NULL,
  `param_value` varchar(64) NOT NULL,
  PRIMARY KEY  (`id`),
  UNIQUE KEY `unique_gw_param` (`d_gw_id`,`param_name`)
) ENGINE=InnoDB AUTO_INCREMENT=10 DEFAULT CHARSET=latin1;

--
-- Dumping data for table `directory_gateway_params`
--

LOCK TABLES `directory_gateway_params` WRITE;
/*!40000 ALTER TABLE `directory_gateway_params` DISABLE KEYS */;
/*!40000 ALTER TABLE `directory_gateway_params` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `directory_gateways`
--

DROP TABLE IF EXISTS `directory_gateways`;
CREATE TABLE `directory_gateways` (
  `id` int(10) unsigned NOT NULL auto_increment,
  `directory_id` int(10) unsigned NOT NULL,
  `gateway_name` varchar(128) NOT NULL,
  PRIMARY KEY  (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=4 DEFAULT CHARSET=latin1;

--
-- Dumping data for table `directory_gateways`
--

LOCK TABLES `directory_gateways` WRITE;
/*!40000 ALTER TABLE `directory_gateways` DISABLE KEYS */;
/*!40000 ALTER TABLE `directory_gateways` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `directory_global_params`
--

DROP TABLE IF EXISTS `directory_global_params`;
CREATE TABLE `directory_global_params` (
  `id` int(10) unsigned NOT NULL auto_increment,
  `param_name` varchar(64) NOT NULL,
  `param_value` varchar(128) NOT NULL,
  `domain_id` int(10) unsigned NOT NULL,
  PRIMARY KEY  (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=2 DEFAULT CHARSET=latin1;

--
-- Dumping data for table `directory_global_params`
--

LOCK TABLES `directory_global_params` WRITE;
/*!40000 ALTER TABLE `directory_global_params` DISABLE KEYS */;
INSERT INTO `directory_global_params` VALUES (1,'default_gateway','errors',1);
/*!40000 ALTER TABLE `directory_global_params` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `directory_global_vars`
--

DROP TABLE IF EXISTS `directory_global_vars`;
CREATE TABLE `directory_global_vars` (
  `id` int(10) unsigned NOT NULL auto_increment,
  `var_name` varchar(64) NOT NULL,
  `var_value` varchar(128) NOT NULL,
  `domain_id` int(10) unsigned NOT NULL,
  PRIMARY KEY  (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Dumping data for table `directory_global_vars`
--

LOCK TABLES `directory_global_vars` WRITE;
/*!40000 ALTER TABLE `directory_global_vars` DISABLE KEYS */;
/*!40000 ALTER TABLE `directory_global_vars` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `directory_params`
--

DROP TABLE IF EXISTS `directory_params`;
CREATE TABLE `directory_params` (
  `id` int(11) NOT NULL auto_increment,
  `directory_id` int(11) default NULL,
  `param_name` varchar(255) default NULL,
  `param_value` varchar(255) default NULL,
  PRIMARY KEY  (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=23 DEFAULT CHARSET=utf8;

--
-- Dumping data for table `directory_params`
--

LOCK TABLES `directory_params` WRITE;
/*!40000 ALTER TABLE `directory_params` DISABLE KEYS */;
INSERT INTO `directory_params` VALUES (1,1,'password','1234'),(2,1,'vm-password','861000'),(3,2,'password','1234'),(4,2,'vm-password','861001'),(7,5,'password','1234'),(8,6,'password','1234'),(9,7,'password','1234'),(10,8,'password','1234'),(11,9,'password','1234'),(12,10,'password','1234'),(13,11,'password','1234'),(14,3,'vm-password','861002'),(15,3,'password','1234'),(16,11,'vm-password','861009'),(17,10,'vm-password','861008'),(18,9,'vm-password','861007'),(19,8,'vm-password','861006'),(20,7,'vm-password','861005'),(21,6,'vm-password','861004'),(22,5,'vm-password','861003');
/*!40000 ALTER TABLE `directory_params` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `directory_vars`
--

DROP TABLE IF EXISTS `directory_vars`;
CREATE TABLE `directory_vars` (
  `id` int(11) NOT NULL auto_increment,
  `directory_id` int(11) default NULL,
  `var_name` varchar(255) default NULL,
  `var_value` varchar(255) default NULL,
  PRIMARY KEY  (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=6 DEFAULT CHARSET=utf8;

--
-- Dumping data for table `directory_vars`
--

LOCK TABLES `directory_vars` WRITE;
/*!40000 ALTER TABLE `directory_vars` DISABLE KEYS */;
INSERT INTO `directory_vars` VALUES (1,1,'numbering_plan','US'),(2,2,'numbering_plan','US'),(3,3,'numbering_plan','AU'),(4,5,'numbering_plan','US'),(5,5,'area_code','434');
/*!40000 ALTER TABLE `directory_vars` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `iax_conf`
--

DROP TABLE IF EXISTS `iax_conf`;
CREATE TABLE `iax_conf` (
  `id` int(11) NOT NULL auto_increment,
  `profile_name` varchar(255) default NULL,
  PRIMARY KEY  (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=4 DEFAULT CHARSET=latin1;

--
-- Dumping data for table `iax_conf`
--

LOCK TABLES `iax_conf` WRITE;
/*!40000 ALTER TABLE `iax_conf` DISABLE KEYS */;
INSERT INTO `iax_conf` VALUES (3,'test_profile');
/*!40000 ALTER TABLE `iax_conf` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `iax_settings`
--

DROP TABLE IF EXISTS `iax_settings`;
CREATE TABLE `iax_settings` (
  `id` int(11) NOT NULL auto_increment,
  `iax_id` int(11) default NULL,
  `param_name` varchar(255) default NULL,
  `param_value` varchar(255) default NULL,
  PRIMARY KEY  (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=43 DEFAULT CHARSET=latin1;

--
-- Dumping data for table `iax_settings`
--

LOCK TABLES `iax_settings` WRITE;
/*!40000 ALTER TABLE `iax_settings` DISABLE KEYS */;
INSERT INTO `iax_settings` VALUES (35,3,'debug','1'),(36,3,'ip','$${local_ip_v4}'),(37,3,'port','4569'),(38,3,'context','public'),(39,3,'dialplan','enum,XML'),(40,3,'codec-prefs','$${global_codec_prefs}'),(41,3,'codec-master','us'),(42,3,'codec-rate','8');
/*!40000 ALTER TABLE `iax_settings` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `ivr_conf`
--

DROP TABLE IF EXISTS `ivr_conf`;
CREATE TABLE `ivr_conf` (
  `id` int(10) unsigned NOT NULL auto_increment,
  `name` varchar(64) NOT NULL,
  `greet_long` varchar(255) NOT NULL,
  `greet_short` varchar(255) NOT NULL,
  `invalid_sound` varchar(255) NOT NULL,
  `exit_sound` varchar(255) NOT NULL,
  `max_failures` int(10) unsigned NOT NULL default '3',
  `timeout` int(11) NOT NULL default '5',
  `tts_engine` varchar(64) default NULL,
  `tts_voice` varchar(64) default NULL,
  PRIMARY KEY  (`id`),
  UNIQUE KEY `unique_name` (`name`)
) ENGINE=InnoDB AUTO_INCREMENT=4 DEFAULT CHARSET=latin1;

--
-- Dumping data for table `ivr_conf`
--

LOCK TABLES `ivr_conf` WRITE;
/*!40000 ALTER TABLE `ivr_conf` DISABLE KEYS */;
INSERT INTO `ivr_conf` VALUES (1,'demo','soundfiles/ivr/demo/greet-long.wav','soundfiles/ivr/demo/greet-short.wav','soundfiles/ivr/invalid.wav','soundfiles/ivr/exit.wav',3,5,'cepstral','allison'),(2,'demo2','soundfiles/ivr/demo2/greet-long.wav','soundfiles/ivr/demo2/greet-short.wav','soundfiles/ivr/invalid.wav','soundfiles/ivr/exit.wav',3,5,NULL,NULL),(3,'menu8','soundfiles/ivr/menu8/greet-long.wav','soundfiles/ivr/menu8/greet-short.wav','soundfiles/ivr/menu8/invalid.wav','soundfiles/ivr/menu8/exit.wav',3,5,NULL,NULL);
/*!40000 ALTER TABLE `ivr_conf` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `ivr_entries`
--

DROP TABLE IF EXISTS `ivr_entries`;
CREATE TABLE `ivr_entries` (
  `id` int(10) unsigned NOT NULL auto_increment,
  `ivr_id` int(10) unsigned NOT NULL,
  `action` varchar(64) NOT NULL,
  `digits` varchar(16) NOT NULL,
  `params` varchar(255) default NULL,
  PRIMARY KEY  (`id`),
  UNIQUE KEY `unique_ivr_digits` USING BTREE (`ivr_id`,`digits`)
) ENGINE=InnoDB AUTO_INCREMENT=11 DEFAULT CHARSET=latin1;

--
-- Dumping data for table `ivr_entries`
--

LOCK TABLES `ivr_entries` WRITE;
/*!40000 ALTER TABLE `ivr_entries` DISABLE KEYS */;
INSERT INTO `ivr_entries` VALUES (1,1,'menu-play-sound','1','soundfiles/features.wav'),(2,1,'menu-exit','*',NULL),(3,1,'menu-sub','2','demo2'),(4,1,'menu-exec-api','3','bridge sofia/$${domain}/888@conference.freeswtich.org'),(5,1,'menu-call-transfer','4','888'),(6,2,'menu-back','#',NULL),(7,2,'menu-top','*',NULL),(8,3,'menu-back','#',NULL),(9,3,'menu-top','*',NULL),(10,3,'menu-playsound','4','soundfiles/4.wav');
/*!40000 ALTER TABLE `ivr_entries` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `limit_conf`
--

DROP TABLE IF EXISTS `limit_conf`;
CREATE TABLE `limit_conf` (
  `id` int(11) NOT NULL auto_increment,
  `name` varchar(255) default NULL,
  `value` varchar(255) default NULL,
  PRIMARY KEY  (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=2 DEFAULT CHARSET=utf8;

--
-- Dumping data for table `limit_conf`
--

LOCK TABLES `limit_conf` WRITE;
/*!40000 ALTER TABLE `limit_conf` DISABLE KEYS */;
/*!40000 ALTER TABLE `limit_conf` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `limit_data`
--

DROP TABLE IF EXISTS `limit_data`;
CREATE TABLE `limit_data` (
  `hostname` varchar(255) default NULL,
  `realm` varchar(255) default NULL,
  `id` varchar(255) default NULL,
  `uuid` varchar(255) default NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Dumping data for table `limit_data`
--

LOCK TABLES `limit_data` WRITE;
/*!40000 ALTER TABLE `limit_data` DISABLE KEYS */;
/*!40000 ALTER TABLE `limit_data` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `local_stream_conf`
--

DROP TABLE IF EXISTS `local_stream_conf`;
CREATE TABLE `local_stream_conf` (
  `id` int(11) NOT NULL auto_increment,
  `directory_name` varchar(255) default NULL,
  `directory_path` text,
  `param_name` varchar(255) default NULL,
  `param_value` varchar(255) default NULL,
  PRIMARY KEY  (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Dumping data for table `local_stream_conf`
--

LOCK TABLES `local_stream_conf` WRITE;
/*!40000 ALTER TABLE `local_stream_conf` DISABLE KEYS */;
/*!40000 ALTER TABLE `local_stream_conf` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `post_load_modules_conf`
--

DROP TABLE IF EXISTS `post_load_modules_conf`;
CREATE TABLE `post_load_modules_conf` (
  `id` int(10) unsigned NOT NULL auto_increment,
  `module_name` varchar(64) NOT NULL,
  `load_module` tinyint(1) NOT NULL default '1',
  `priority` int(10) unsigned NOT NULL default '1000',
  PRIMARY KEY  (`id`),
  UNIQUE KEY `unique_mod` (`module_name`)
) ENGINE=InnoDB AUTO_INCREMENT=54 DEFAULT CHARSET=latin1;

--
-- Dumping data for table `post_load_modules_conf`
--

LOCK TABLES `post_load_modules_conf` WRITE;
/*!40000 ALTER TABLE `post_load_modules_conf` DISABLE KEYS */;
INSERT INTO `post_load_modules_conf` VALUES (1,'mod_sofia',1,2000),(2,'mod_iax',1,2000),(3,'mod_xml_rpc',1,100),(4,'mod_portaudio',1,1000),(5,'mod_enum',1,2000),(6,'mod_xml_cdr',1,1000),(7,'mod_spidermonkey',1,1000),(8,'mod_alsa',0,1000),(9,'mod_log_file',1,0),(10,'mod_commands',1,1000),(11,'mod_voicemail',1,1000),(12,'mod_dialplan_xml',1,150),(13,'mod_dialplan_asterisk',1,150),(14,'mod_openzap',0,1000),(15,'mod_woomera',0,1000),(17,'mod_speex',1,500),(18,'mod_ilbc',0,1000),(20,'mod_g723_1',1,500),(21,'mod_g729',1,500),(22,'mod_g722',1,500),(23,'mod_g726',1,500),(25,'mod_amr',1,500),(26,'mod_fifo',1,1000),(27,'mod_limit',1,1000),(28,'mod_syslog',1,0),(29,'mod_dingaling',1,2000),(30,'mod_cdr_csv',1,1000),(31,'mod_event_socket',1,100),(32,'mod_multicast',0,1000),(33,'mod_zeroconf',0,1000),(34,'mod_xmpp_event',0,1000),(35,'mod_sndfile',1,1000),(36,'mod_native_file',1,1000),(37,'mod_shout',1,1000),(38,'mod_local_stream',1,1000),(39,'mod_perl',0,1000),(40,'mod_python',0,1000),(41,'mod_java',0,1000),(42,'mod_cepstral',0,1000),(43,'mod_openmrcp',0,1000),(44,'mod_lumenvox',0,1000),(45,'mod_rss',0,1000),(46,'mod_say_de',1,1000),(47,'mod_say_fr',0,1000),(48,'mod_say_en',1,1000),(49,'mod_conference',1,1000),(50,'mod_ivr',0,1000),(51,'mod_console',1,0),(52,'mod_dptools',1,1500),(53,'mod_voipcodecs',1,500);
/*!40000 ALTER TABLE `post_load_modules_conf` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `rss_conf`
--

DROP TABLE IF EXISTS `rss_conf`;
CREATE TABLE `rss_conf` (
  `id` int(11) NOT NULL auto_increment,
  `directory_id` int(11) NOT NULL,
  `feed` text NOT NULL,
  `local_file` text NOT NULL,
  `description` text,
  `priority` int(11) NOT NULL default '1000',
  PRIMARY KEY  (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=15 DEFAULT CHARSET=utf8;

--
-- Dumping data for table `rss_conf`
--

LOCK TABLES `rss_conf` WRITE;
/*!40000 ALTER TABLE `rss_conf` DISABLE KEYS */;
/*!40000 ALTER TABLE `rss_conf` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `sofia_aliases`
--

DROP TABLE IF EXISTS `sofia_aliases`;
CREATE TABLE `sofia_aliases` (
  `id` int(10) unsigned NOT NULL auto_increment,
  `sofia_id` int(10) unsigned NOT NULL,
  `alias_name` varchar(255) NOT NULL,
  PRIMARY KEY  (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=5 DEFAULT CHARSET=latin1;

--
-- Dumping data for table `sofia_aliases`
--

LOCK TABLES `sofia_aliases` WRITE;
/*!40000 ALTER TABLE `sofia_aliases` DISABLE KEYS */;
INSERT INTO `sofia_aliases` VALUES (1,1,'default'),(3,1,'sip.example.com');
/*!40000 ALTER TABLE `sofia_aliases` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `sofia_conf`
--

DROP TABLE IF EXISTS `sofia_conf`;
CREATE TABLE `sofia_conf` (
  `id` int(11) NOT NULL auto_increment,
  `profile_name` varchar(255) default NULL,
  PRIMARY KEY  (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=3 DEFAULT CHARSET=latin1;

--
-- Dumping data for table `sofia_conf`
--

LOCK TABLES `sofia_conf` WRITE;
/*!40000 ALTER TABLE `sofia_conf` DISABLE KEYS */;
INSERT INTO `sofia_conf` VALUES (1,'$${domain}');
/*!40000 ALTER TABLE `sofia_conf` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `sofia_domains`
--

DROP TABLE IF EXISTS `sofia_domains`;
CREATE TABLE `sofia_domains` (
  `id` int(11) NOT NULL auto_increment,
  `sofia_id` int(11) default NULL,
  `domain_name` varchar(255) default NULL,
  `parse` tinyint(1) default NULL,
  PRIMARY KEY  (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Dumping data for table `sofia_domains`
--

LOCK TABLES `sofia_domains` WRITE;
/*!40000 ALTER TABLE `sofia_domains` DISABLE KEYS */;
/*!40000 ALTER TABLE `sofia_domains` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `sofia_gateways`
--

DROP TABLE IF EXISTS `sofia_gateways`;
CREATE TABLE `sofia_gateways` (
  `id` int(11) NOT NULL auto_increment,
  `sofia_id` int(11) default NULL,
  `gateway_name` varchar(255) default NULL,
  `gateway_param` varchar(255) default NULL,
  `gateway_value` varchar(255) default NULL,
  PRIMARY KEY  (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=15 DEFAULT CHARSET=latin1;

--
-- Dumping data for table `sofia_gateways`
--

LOCK TABLES `sofia_gateways` WRITE;
/*!40000 ALTER TABLE `sofia_gateways` DISABLE KEYS */;
INSERT INTO `sofia_gateways` VALUES (8,1,'default','proxy','asterlink.com'),(9,1,'default','realm','asterlink.com'),(10,1,'default','username','USERNAME_HERE'),(11,1,'default','register','false'),(12,1,'default','expire-seconds','60'),(13,1,'default','retry_seconds','2'),(14,1,'default','password','PASSWORD_HERE');
/*!40000 ALTER TABLE `sofia_gateways` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `sofia_settings`
--

DROP TABLE IF EXISTS `sofia_settings`;
CREATE TABLE `sofia_settings` (
  `id` int(11) NOT NULL auto_increment,
  `sofia_id` int(11) default NULL,
  `param_name` varchar(255) default NULL,
  `param_value` varchar(255) default NULL,
  PRIMARY KEY  (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=37 DEFAULT CHARSET=latin1;

--
-- Dumping data for table `sofia_settings`
--

LOCK TABLES `sofia_settings` WRITE;
/*!40000 ALTER TABLE `sofia_settings` DISABLE KEYS */;
INSERT INTO `sofia_settings` VALUES (1,1,'user-agent-string','RayUA 2.0pre4'),(2,1,'auth-calls','true'),(5,1,'debug','1'),(6,1,'rfc2833-pt','101'),(7,1,'sip-port','5060'),(8,1,'dialplan','XML'),(9,1,'dtmf-duration','100'),(10,1,'codec-prefs','$${global_codec_prefs}'),(11,1,'rtp-timeout-sec','300'),(12,1,'rtp-ip','$${local_ip_v4}'),(13,1,'sip-ip','$${local_ip_v4}'),(14,1,'context','default'),(15,1,'manage-presence','true'),(16,1,'force-register-domain','intralanman.servehttp.com'),(17,1,'inbound-codec-negotiation','generous'),(18,1,'rtp-rewrite-timestampes','true'),(19,1,'nonce-ttl','60'),(20,1,'vad','out'),(36,1,'odbc-dsn','freeswitch-mysql:freeswitch:Fr33Sw1tch');
/*!40000 ALTER TABLE `sofia_settings` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `voicemail_conf`
--

DROP TABLE IF EXISTS `voicemail_conf`;
CREATE TABLE `voicemail_conf` (
  `id` int(10) unsigned NOT NULL auto_increment,
  `vm_profile` varchar(64) NOT NULL,
  PRIMARY KEY  (`id`),
  UNIQUE KEY `unique_profile` (`vm_profile`)
) ENGINE=InnoDB AUTO_INCREMENT=3 DEFAULT CHARSET=latin1;

--
-- Dumping data for table `voicemail_conf`
--

LOCK TABLES `voicemail_conf` WRITE;
/*!40000 ALTER TABLE `voicemail_conf` DISABLE KEYS */;
INSERT INTO `voicemail_conf` VALUES (1,'default');
/*!40000 ALTER TABLE `voicemail_conf` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `voicemail_email`
--

DROP TABLE IF EXISTS `voicemail_email`;
CREATE TABLE `voicemail_email` (
  `id` int(10) unsigned NOT NULL auto_increment,
  `voicemail_id` int(10) unsigned NOT NULL,
  `param_name` varchar(64) NOT NULL,
  `param_value` varchar(64) NOT NULL,
  PRIMARY KEY  (`id`),
  UNIQUE KEY `unique_profile_param` (`param_name`,`voicemail_id`)
) ENGINE=InnoDB AUTO_INCREMENT=4 DEFAULT CHARSET=latin1;

--
-- Dumping data for table `voicemail_email`
--

LOCK TABLES `voicemail_email` WRITE;
/*!40000 ALTER TABLE `voicemail_email` DISABLE KEYS */;
INSERT INTO `voicemail_email` VALUES (1,1,'template-file','voicemail.tpl'),(2,1,'date-fmt','%A, %B %d %Y, %I %M %p'),(3,1,'email-from','${voicemail_account}@${voicemail_domain}');
/*!40000 ALTER TABLE `voicemail_email` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `voicemail_settings`
--

DROP TABLE IF EXISTS `voicemail_settings`;
CREATE TABLE `voicemail_settings` (
  `id` int(11) NOT NULL auto_increment,
  `voicemail_id` int(11) default NULL,
  `param_name` varchar(255) default NULL,
  `param_value` varchar(255) default NULL,
  PRIMARY KEY  (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=31 DEFAULT CHARSET=latin1;

--
-- Dumping data for table `voicemail_settings`
--

LOCK TABLES `voicemail_settings` WRITE;
/*!40000 ALTER TABLE `voicemail_settings` DISABLE KEYS */;
INSERT INTO `voicemail_settings` VALUES (1,1,'file-extension','wav'),(2,1,'terminator-key','#'),(3,1,'max-login-attempts','3'),(4,1,'digit-timeout','10000'),(5,1,'max-record-length','300'),(6,1,'tone-spec','%(1000, 0, 640)'),(7,1,'callback-dialplan','XML'),(8,1,'callback-context','default'),(9,1,'play-new-messages-key','1'),(10,1,'play-saved-messages-key','2'),(11,1,'main-menu-key','*'),(12,1,'config-menu-key','5'),(13,1,'record-greeting-key','1'),(14,1,'choose-greeting-key','2'),(15,1,'record-file-key','3'),(16,1,'listen-file-key','1'),(17,1,'record-name-key','3'),(18,1,'save-file-key','9'),(19,1,'delete-file-key','7'),(20,1,'undelete-file-key','8'),(21,1,'email-key','4'),(22,1,'pause-key','0'),(23,1,'restart-key','1'),(24,1,'ff-key','6'),(25,1,'rew-key','4'),(26,1,'record-silence-threshold','200'),(27,1,'record-silence-hits','2'),(28,1,'web-template-file','web-vm.tpl'),(29,1,'operator-extension','operator XML default'),(30,1,'operator-key','9');
/*!40000 ALTER TABLE `voicemail_settings` ENABLE KEYS */;
UNLOCK TABLES;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

-- Dump completed on 2008-03-25  4:57:01
