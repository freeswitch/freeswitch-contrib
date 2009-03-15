-- MySQL dump 10.11
--
-- Host: localhost    Database: shipment
-- ------------------------------------------------------
-- Server version	5.0.45

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
-- Table structure for table `domain_params`
--

DROP TABLE IF EXISTS `domain_params`;
CREATE TABLE `domain_params` (
  `uid` int(11) NOT NULL auto_increment,
  `domains_uid` int(11) NOT NULL,
  `name` varchar(64) NOT NULL,
  `value` varchar(256) NOT NULL,
  PRIMARY KEY  (`uid`),
  KEY `domain_uid` (`domains_uid`)
) ENGINE=MyISAM AUTO_INCREMENT=2 DEFAULT CHARSET=latin1;

--
-- Dumping data for table `domain_params`
--

LOCK TABLES `domain_params` WRITE;
/*!40000 ALTER TABLE `domain_params` DISABLE KEYS */;
INSERT INTO `domain_params` VALUES (1,1,'dial-string','{presence_id=${dialed_user}@${dialed_domain}}${sofia_contact(${dialed_user}@${dialed_domain})}');
/*!40000 ALTER TABLE `domain_params` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `domain_variables`
--

DROP TABLE IF EXISTS `domain_variables`;
CREATE TABLE `domain_variables` (
  `uid` int(11) NOT NULL auto_increment,
  `domains_uid` int(11) NOT NULL,
  `name` varchar(64) NOT NULL,
  `value` varchar(128) NOT NULL,
  PRIMARY KEY  (`uid`),
  KEY `domain_uid` (`domains_uid`)
) ENGINE=MyISAM AUTO_INCREMENT=5 DEFAULT CHARSET=latin1;

--
-- Dumping data for table `domain_variables`
--

LOCK TABLES `domain_variables` WRITE;
/*!40000 ALTER TABLE `domain_variables` DISABLE KEYS */;
INSERT INTO `domain_variables` VALUES (1,1,'record_stereo','true'),(2,1,'default_gateway','$${default_provider}'),(3,1,'default_areacode','$${default_areacode}'),(4,1,'transfer_fallback_extension','operator');
/*!40000 ALTER TABLE `domain_variables` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `domains`
--

DROP TABLE IF EXISTS `domains`;
CREATE TABLE `domains` (
  `uid` int(11) NOT NULL auto_increment COMMENT 'UID for the table Auto Assigned',
  `name` varchar(128) NOT NULL COMMENT 'domaine name',
  `enabled` tinyint(1) NOT NULL default '1',
  PRIMARY KEY  (`uid`),
  UNIQUE KEY `name` (`name`)
) ENGINE=MyISAM AUTO_INCREMENT=2 DEFAULT CHARSET=latin1;

--
-- Dumping data for table `domains`
--

LOCK TABLES `domains` WRITE;
/*!40000 ALTER TABLE `domains` DISABLE KEYS */;
INSERT INTO `domains` VALUES (1,'192.168.1.140',1);
/*!40000 ALTER TABLE `domains` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `group_members`
--

DROP TABLE IF EXISTS `group_members`;
CREATE TABLE `group_members` (
  `uid` int(11) NOT NULL auto_increment,
  `domains_uid` int(11) NOT NULL,
  `groups_uid` int(11) NOT NULL,
  `users_uid` int(11) NOT NULL,
  `type` varchar(32) NOT NULL,
  PRIMARY KEY  (`uid`),
  KEY `domain_uid` (`domains_uid`,`groups_uid`,`users_uid`)
) ENGINE=MyISAM AUTO_INCREMENT=13 DEFAULT CHARSET=latin1;

--
-- Dumping data for table `group_members`
--

LOCK TABLES `group_members` WRITE;
/*!40000 ALTER TABLE `group_members` DISABLE KEYS */;
INSERT INTO `group_members` VALUES (1,1,1,1,'pointer'),(2,1,1,2,'pointer'),(3,1,1,3,'pointer'),(4,1,1,4,'pointer'),(5,1,2,5,'pointer'),(6,1,2,6,'pointer'),(7,1,2,7,'pointer'),(8,1,2,8,'pointer'),(9,1,3,9,'pointer'),(10,1,3,10,'pointer'),(11,1,3,11,'pointer'),(12,1,3,12,'pointer');
/*!40000 ALTER TABLE `group_members` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `groups`
--

DROP TABLE IF EXISTS `groups`;
CREATE TABLE `groups` (
  `uid` int(11) NOT NULL auto_increment,
  `domains_uid` int(11) NOT NULL,
  `name` varchar(64) NOT NULL,
  PRIMARY KEY  (`uid`),
  KEY `domain_uid` (`domains_uid`)
) ENGINE=MyISAM AUTO_INCREMENT=4 DEFAULT CHARSET=latin1;

--
-- Dumping data for table `groups`
--

LOCK TABLES `groups` WRITE;
/*!40000 ALTER TABLE `groups` DISABLE KEYS */;
INSERT INTO `groups` VALUES (1,1,'sales'),(2,1,'billing'),(3,1,'support');
/*!40000 ALTER TABLE `groups` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `menu_entries`
--

DROP TABLE IF EXISTS `menu_entries`;
CREATE TABLE `menu_entries` (
  `uid` int(11) NOT NULL auto_increment,
  `menu_uid` int(11) NOT NULL,
  `action` varchar(32) NOT NULL,
  `digits` varchar(16) NOT NULL,
  `param` varchar(256) default NULL,
  PRIMARY KEY  (`uid`)
) ENGINE=MyISAM AUTO_INCREMENT=9 DEFAULT CHARSET=latin1;

--
-- Dumping data for table `menu_entries`
--

LOCK TABLES `menu_entries` WRITE;
/*!40000 ALTER TABLE `menu_entries` DISABLE KEYS */;
INSERT INTO `menu_entries` VALUES (1,1,'menu-exec-app','1','bridge sofia/$${domain}/888@conference.freeswitch.org'),(2,1,'menu-exec-app','2','transfer 9996 XML default'),(3,1,'menu-exec-app','3','transfer 9999 XML default'),(4,1,'menu-sub','4','demo_ivr_submenu'),(5,1,'menu-exec-app','5','transfer 1234*256 enum'),(6,1,'menu-exec-app','/^(10[01][0-9])$','transfer $1 XML features'),(7,1,'menu-top','9',NULL),(8,2,'menu-top','*',NULL);
/*!40000 ALTER TABLE `menu_entries` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `menus`
--

DROP TABLE IF EXISTS `menus`;
CREATE TABLE `menus` (
  `uid` int(11) NOT NULL auto_increment,
  `name` varchar(64) NOT NULL,
  `greet_long` varchar(256) default NULL,
  `greet_short` varchar(256) default NULL,
  `invalid_sound` varchar(256) default NULL,
  `exit_sound` varchar(256) default NULL,
  `timeout` int(11) default NULL,
  `inter_digit_timeout` int(11) default NULL,
  `max_failures` int(11) default NULL,
  `max_timeouts` int(11) default NULL,
  `digit_len` int(11) default NULL,
  PRIMARY KEY  (`uid`)
) ENGINE=MyISAM AUTO_INCREMENT=3 DEFAULT CHARSET=latin1;

--
-- Dumping data for table `menus`
--

LOCK TABLES `menus` WRITE;
/*!40000 ALTER TABLE `menus` DISABLE KEYS */;
INSERT INTO `menus` VALUES (1,'demo_ivr','phrase:demo_ivr_main_menu','phrase:demo_ivr_main_menu_short','ivr/ivr-that_was_an_invalid_entry.wav','voicemail/vm-goodbye.wav',10000,2000,3,3,4),(2,'demo_ivr_submenu','phrase:demo_ivr_sub_menu','phrase:demo_ivr_sub_menu_short','ivr/ivr-that_was_an_invalid_entry.wav','voicemail/vm-goodbye.wav',15000,NULL,3,3,NULL);
/*!40000 ALTER TABLE `menus` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `user_params`
--

DROP TABLE IF EXISTS `user_params`;
CREATE TABLE `user_params` (
  `uid` int(11) NOT NULL auto_increment,
  `users_uid` int(11) NOT NULL,
  `name` varchar(64) NOT NULL,
  `value` varchar(128) NOT NULL,
  PRIMARY KEY  (`uid`),
  KEY `user_uid` (`users_uid`)
) ENGINE=MyISAM AUTO_INCREMENT=39 DEFAULT CHARSET=latin1;

--
-- Dumping data for table `user_params`
--

LOCK TABLES `user_params` WRITE;
/*!40000 ALTER TABLE `user_params` DISABLE KEYS */;
INSERT INTO `user_params` VALUES (1,1,'password','1234'),(2,1,'vm-password','1000'),(3,2,'password','1234'),(4,2,'vm-password','1002'),(5,3,'password','1234'),(6,3,'vm-password','1003'),(7,4,'password','1234'),(8,4,'vm-password','1004'),(9,5,'password','1234'),(10,5,'vm-password','1005'),(11,6,'password','1234'),(12,6,'vm-password','1006'),(13,7,'password','1234'),(14,7,'vm-password','1007'),(15,8,'password','1234'),(16,8,'vm-password','1008'),(17,9,'password','1234'),(18,9,'vm-password','1009'),(19,10,'password','1234'),(20,10,'vm-password','1010'),(21,11,'password','1234'),(22,11,'vm-password','1011'),(23,12,'password','1234'),(24,12,'vm-password','1012'),(25,13,'password','1234'),(26,13,'vm-password','1013'),(27,14,'password','1234'),(28,14,'vm-password','1014'),(29,15,'password','1234'),(30,15,'vm-password','1015'),(31,16,'password','1234'),(32,16,'vm-password','1016'),(33,17,'password','1234'),(34,17,'vm-password','1017'),(35,18,'password','1234'),(36,18,'vm-password','1018'),(37,19,'password','1234'),(38,19,'vm-password','1019');
/*!40000 ALTER TABLE `user_params` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `user_variables`
--

DROP TABLE IF EXISTS `user_variables`;
CREATE TABLE `user_variables` (
  `uid` int(11) NOT NULL auto_increment,
  `users_uid` int(11) NOT NULL,
  `name` varchar(64) NOT NULL,
  `value` varchar(128) NOT NULL,
  PRIMARY KEY  (`uid`),
  KEY `user_uid` (`users_uid`)
) ENGINE=MyISAM AUTO_INCREMENT=9 DEFAULT CHARSET=latin1;

--
-- Dumping data for table `user_variables`
--

LOCK TABLES `user_variables` WRITE;
/*!40000 ALTER TABLE `user_variables` DISABLE KEYS */;
INSERT INTO `user_variables` VALUES (1,1,'toll_allow','domestic,international,local'),(2,1,'accountcode','1000'),(3,1,'user_context','default'),(4,1,'effective_caller_id_name','Extension 1000'),(5,1,'effective_caller_id_number','1000'),(6,1,'outbound_caller_id_name','$${outbound_caller_name}'),(7,1,'outbound_caller_id_number','$${outbound_caller_id}'),(8,1,'callgroup','techsupport');
/*!40000 ALTER TABLE `user_variables` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `users`
--

DROP TABLE IF EXISTS `users`;
CREATE TABLE `users` (
  `uid` int(11) NOT NULL auto_increment,
  `domain_uid` int(11) NOT NULL,
  `username` varchar(64) NOT NULL,
  `mailbox` varchar(64) default NULL,
  `cidr` varchar(32) default NULL,
  `enabled` tinyint(1) NOT NULL default '1',
  PRIMARY KEY  (`uid`),
  KEY `domain_uid` (`domain_uid`,`username`,`mailbox`)
) ENGINE=MyISAM AUTO_INCREMENT=21 DEFAULT CHARSET=latin1;

--
-- Dumping data for table `users`
--

LOCK TABLES `users` WRITE;
/*!40000 ALTER TABLE `users` DISABLE KEYS */;
INSERT INTO `users` VALUES (1,1,'1000','1000',NULL,1),(2,1,'1001','1001',NULL,1),(3,1,'1002','1002',NULL,1),(4,1,'1003','1003',NULL,1),(5,1,'1004','1004',NULL,1),(6,1,'1005','1005',NULL,1),(7,1,'1006','1006',NULL,1),(8,1,'1007','1007',NULL,1),(9,1,'1008','1008',NULL,1),(10,1,'1009','1009',NULL,1),(11,1,'1010','1010',NULL,1),(12,1,'1011','1011',NULL,1),(13,1,'1012','1012',NULL,1),(14,1,'1013','1013',NULL,1),(15,1,'1014','1014',NULL,1),(16,1,'1015','1015',NULL,1),(17,1,'1016','1016',NULL,1),(18,1,'1017','1017',NULL,1),(19,1,'1018','1018',NULL,1),(20,1,'1019','1019',NULL,1);
/*!40000 ALTER TABLE `users` ENABLE KEYS */;
UNLOCK TABLES;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

-- Dump completed on 2009-03-15  9:30:01
