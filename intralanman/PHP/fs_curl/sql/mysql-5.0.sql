-- MySQL dump 10.11
--
-- Host: localhost    Database: freeswitch
-- ------------------------------------------------------
-- Server version	5.0.32-Debian_7etch5-log

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
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

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
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

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
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Table structure for table `db_data`
--

DROP TABLE IF EXISTS `db_data`;
CREATE TABLE `db_data` (
  `hostname` varchar(255) default NULL,
  `realm` varchar(255) default NULL,
  `data_key` varchar(255) default NULL,
  `data` varchar(255) default NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

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
  `condition_continue` varchar(8) default NULL,
  `app_cdata` tinyint(1) NOT NULL default '0',
  `re_cdata` tinyint(1) NOT NULL default '0',
  PRIMARY KEY  (`id`)
) ENGINE=MyISAM AUTO_INCREMENT=386 DEFAULT CHARSET=utf8;

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
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

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
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

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
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Table structure for table `directory`
--

DROP TABLE IF EXISTS `directory`;
CREATE TABLE `directory` (
  `id` int(11) NOT NULL auto_increment,
  `username` varchar(255) NOT NULL,
  `domain` varchar(255) NOT NULL,
  PRIMARY KEY  (`id`)
) ENGINE=MyISAM AUTO_INCREMENT=5 DEFAULT CHARSET=utf8;

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
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Table structure for table `directory_gateways`
--

DROP TABLE IF EXISTS `directory_gateways`;
CREATE TABLE `directory_gateways` (
  `id` int(10) unsigned NOT NULL auto_increment,
  `directory_id` int(10) unsigned NOT NULL,
  `gateway_name` varchar(128) NOT NULL,
  PRIMARY KEY  (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

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
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

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
) ENGINE=MyISAM AUTO_INCREMENT=6 DEFAULT CHARSET=utf8;

--
-- Table structure for table `group_data`
--

DROP TABLE IF EXISTS `group_data`;
CREATE TABLE `group_data` (
  `hostname` varchar(255) default NULL,
  `groupname` varchar(255) default NULL,
  `url` varchar(255) default NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Table structure for table `iax_conf`
--

DROP TABLE IF EXISTS `iax_conf`;
CREATE TABLE `iax_conf` (
  `id` int(11) NOT NULL auto_increment,
  `profile_name` varchar(255) default NULL,
  PRIMARY KEY  (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

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
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

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
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

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
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Table structure for table `jabber_subscriptions`
--

DROP TABLE IF EXISTS `jabber_subscriptions`;
CREATE TABLE `jabber_subscriptions` (
  `sub_from` varchar(255) default NULL,
  `sub_to` varchar(255) default NULL,
  `show_pres` varchar(255) default NULL,
  `status` varchar(255) default NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Table structure for table `limit_conf`
--

DROP TABLE IF EXISTS `limit_conf`;
CREATE TABLE `limit_conf` (
  `id` int(11) NOT NULL auto_increment,
  `name` varchar(255) default NULL,
  `value` varchar(255) default NULL,
  PRIMARY KEY  (`id`)
) ENGINE=MyISAM AUTO_INCREMENT=2 DEFAULT CHARSET=utf8;

--
-- Table structure for table `limit_data`
--

DROP TABLE IF EXISTS `limit_data`;
CREATE TABLE `limit_data` (
  `hostname` varchar(255) default NULL,
  `realm` varchar(255) default NULL,
  `id` varchar(255) default NULL,
  `uuid` varchar(255) default NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

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
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

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
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

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
) ENGINE=MyISAM AUTO_INCREMENT=15 DEFAULT CHARSET=utf8;

--
-- Table structure for table `sip_authentication`
--

DROP TABLE IF EXISTS `sip_authentication`;
CREATE TABLE `sip_authentication` (
  `nonce` varchar(255) default NULL,
  `expires` int(8) default NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Table structure for table `sip_dialogs`
--

DROP TABLE IF EXISTS `sip_dialogs`;
CREATE TABLE `sip_dialogs` (
  `call_id` varchar(255) default NULL,
  `uuid` varchar(255) default NULL,
  `sip_to_user` varchar(255) default NULL,
  `sip_to_host` varchar(255) default NULL,
  `sip_from_user` varchar(255) default NULL,
  `sip_from_host` varchar(255) default NULL,
  `contact_user` varchar(255) default NULL,
  `contact_host` varchar(255) default NULL,
  `state` varchar(255) default NULL,
  `direction` varchar(255) default NULL,
  `user_agent` varchar(255) default NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Table structure for table `sip_registrations`
--

DROP TABLE IF EXISTS `sip_registrations`;
CREATE TABLE `sip_registrations` (
  `call_id` varchar(255) default NULL,
  `sip_user` varchar(255) default NULL,
  `sip_host` varchar(255) default NULL,
  `contact` varchar(1024) default NULL,
  `status` varchar(255) default NULL,
  `rpid` varchar(255) default NULL,
  `expires` int(11) default NULL,
  `user_agent` varchar(255) default NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Table structure for table `sip_subscriptions`
--

DROP TABLE IF EXISTS `sip_subscriptions`;
CREATE TABLE `sip_subscriptions` (
  `proto` varchar(255) default NULL,
  `sip_user` varchar(255) default NULL,
  `sip_host` varchar(255) default NULL,
  `sub_to_user` varchar(255) default NULL,
  `sub_to_host` varchar(255) default NULL,
  `event` varchar(255) default NULL,
  `contact` varchar(1024) default NULL,
  `call_id` varchar(255) default NULL,
  `full_from` varchar(255) default NULL,
  `full_via` varchar(255) default NULL,
  `expires` int(11) default NULL,
  `user_agent` varchar(255) default NULL,
  `accept` varchar(255) default NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Table structure for table `sofia_aliases`
--

DROP TABLE IF EXISTS `sofia_aliases`;
CREATE TABLE `sofia_aliases` (
  `id` int(10) unsigned NOT NULL auto_increment,
  `sofia_id` int(10) unsigned NOT NULL,
  `alias_name` varchar(255) NOT NULL,
  PRIMARY KEY  (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Table structure for table `sofia_conf`
--

DROP TABLE IF EXISTS `sofia_conf`;
CREATE TABLE `sofia_conf` (
  `id` int(11) NOT NULL auto_increment,
  `profile_name` varchar(255) default NULL,
  PRIMARY KEY  (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

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
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

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
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Table structure for table `voicemail_conf`
--

DROP TABLE IF EXISTS `voicemail_conf`;
CREATE TABLE `voicemail_conf` (
  `id` int(10) unsigned NOT NULL auto_increment,
  `vm_profile` varchar(64) NOT NULL,
  PRIMARY KEY  (`id`),
  UNIQUE KEY `unique_profile` (`vm_profile`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Table structure for table `voicemail_data`
--

DROP TABLE IF EXISTS `voicemail_data`;
CREATE TABLE `voicemail_data` (
  `created_epoch` int(8) default NULL,
  `read_epoch` int(8) default NULL,
  `user` varchar(255) default NULL,
  `domain` varchar(255) default NULL,
  `uuid` varchar(255) default NULL,
  `cid_name` varchar(255) default NULL,
  `cid_number` varchar(255) default NULL,
  `in_folder` varchar(255) default NULL,
  `file_path` varchar(255) default NULL,
  `flags` varchar(255) default NULL,
  `read_flags` varchar(255) default NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

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
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Table structure for table `voicemail_prefs`
--

DROP TABLE IF EXISTS `voicemail_prefs`;
CREATE TABLE `voicemail_prefs` (
  `user` varchar(255) default NULL,
  `domain` varchar(255) default NULL,
  `name_path` varchar(255) default NULL,
  `greeting_path` varchar(255) default NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

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
) ENGINE=InnoDB DEFAULT CHARSET=latin1;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

-- Dump completed on 2008-03-05  1:16:13
