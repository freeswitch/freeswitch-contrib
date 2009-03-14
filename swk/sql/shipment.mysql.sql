-- 
-- Initial Table with Sample Data for Project Shipment
-- (Or atleast I'm calling it Shipment until I get a better name
-- 

SET SQL_MODE="NO_AUTO_VALUE_ON_ZERO";

--
-- Database: `shipment`
--

-- --------------------------------------------------------

--
-- Table structure for table `domains`
--

CREATE TABLE IF NOT EXISTS `domains` (
  `uid` int(11) NOT NULL auto_increment COMMENT 'UID for the table Auto Assigned',
  `name` varchar(128) NOT NULL COMMENT 'domaine name',
  `enabled` tinyint(1) NOT NULL default '1',
  PRIMARY KEY  (`uid`),
  UNIQUE KEY `name` (`name`)
) ENGINE=MyISAM  DEFAULT CHARSET=latin1 AUTO_INCREMENT=2 ;

--
-- Dumping data for table `domains`
--

INSERT INTO `domains` (`uid`, `name`, `enabled`) VALUES
(1, '192.168.1.140', 1);

-- --------------------------------------------------------

--
-- Table structure for table `domain_params`
--

CREATE TABLE IF NOT EXISTS `domain_params` (
  `uid` int(11) NOT NULL auto_increment,
  `domains_uid` int(11) NOT NULL,
  `name` varchar(64) NOT NULL,
  `value` varchar(256) NOT NULL,
  PRIMARY KEY  (`uid`),
  KEY `domain_uid` (`domains_uid`)
) ENGINE=MyISAM  DEFAULT CHARSET=latin1 AUTO_INCREMENT=2 ;

--
-- Dumping data for table `domain_params`
--

INSERT INTO `domain_params` (`uid`, `domains_uid`, `name`, `value`) VALUES
(1, 1, 'dial-string', '{presence_id=${dialed_user}@${dialed_domain}}${sofia_contact(${dialed_user}@${dialed_domain})}');

-- --------------------------------------------------------

--
-- Table structure for table `domain_variables`
--

CREATE TABLE IF NOT EXISTS `domain_variables` (
  `uid` int(11) NOT NULL auto_increment,
  `domains_uid` int(11) NOT NULL,
  `name` varchar(64) NOT NULL,
  `value` varchar(128) NOT NULL,
  PRIMARY KEY  (`uid`),
  KEY `domain_uid` (`domains_uid`)
) ENGINE=MyISAM  DEFAULT CHARSET=latin1 AUTO_INCREMENT=5 ;

--
-- Dumping data for table `domain_variables`
--

INSERT INTO `domain_variables` (`uid`, `domains_uid`, `name`, `value`) VALUES
(1, 1, 'record_stereo', 'true'),
(2, 1, 'default_gateway', '$${default_provider}'),
(3, 1, 'default_areacode', '$${default_areacode}'),
(4, 1, 'transfer_fallback_extension', 'operator');

-- --------------------------------------------------------

--
-- Table structure for table `groups`
--

CREATE TABLE IF NOT EXISTS `groups` (
  `uid` int(11) NOT NULL auto_increment,
  `domains_uid` int(11) NOT NULL,
  `name` varchar(64) NOT NULL,
  PRIMARY KEY  (`uid`),
  KEY `domain_uid` (`domains_uid`)
) ENGINE=MyISAM  DEFAULT CHARSET=latin1 AUTO_INCREMENT=4 ;

--
-- Dumping data for table `groups`
--

INSERT INTO `groups` (`uid`, `domains_uid`, `name`) VALUES
(1, 1, 'sales'),
(2, 1, 'billing'),
(3, 1, 'support');

-- --------------------------------------------------------

--
-- Table structure for table `group_members`
--

CREATE TABLE IF NOT EXISTS `group_members` (
  `uid` int(11) NOT NULL auto_increment,
  `domains_uid` int(11) NOT NULL,
  `groups_uid` int(11) NOT NULL,
  `users_uid` int(11) NOT NULL,
  `type` varchar(32) NOT NULL,
  PRIMARY KEY  (`uid`),
  KEY `domain_uid` (`domains_uid`,`groups_uid`,`users_uid`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 AUTO_INCREMENT=1 ;

--
-- Dumping data for table `group_members`
--


-- --------------------------------------------------------

--
-- Table structure for table `users`
--

CREATE TABLE IF NOT EXISTS `users` (
  `uid` int(11) NOT NULL auto_increment,
  `domain_uid` int(11) NOT NULL,
  `username` varchar(64) NOT NULL,
  `mailbox` varchar(64) default NULL,
  `cidr` varchar(32) default NULL,
  `enabled` tinyint(1) NOT NULL default '1',
  PRIMARY KEY  (`uid`),
  KEY `domain_uid` (`domain_uid`,`username`,`mailbox`)
) ENGINE=MyISAM  DEFAULT CHARSET=latin1 AUTO_INCREMENT=21 ;

--
-- Dumping data for table `users`
--

INSERT INTO `users` (`uid`, `domain_uid`, `username`, `mailbox`, `cidr`, `enabled`) VALUES
(1, 1, '1000', '1000', NULL, 1),
(2, 1, '1001', '1001', NULL, 1),
(3, 1, '1002', '1002', NULL, 1),
(4, 1, '1003', '1003', NULL, 1),
(5, 1, '1004', '1004', NULL, 1),
(6, 1, '1005', '1005', NULL, 1),
(7, 1, '1006', '1006', NULL, 1),
(8, 1, '1007', '1007', NULL, 1),
(9, 1, '1008', '1008', NULL, 1),
(10, 1, '1009', '1009', NULL, 1),
(11, 1, '1000', '1000', NULL, 1),
(12, 1, '1011', '1011', NULL, 1),
(13, 1, '1012', '1012', NULL, 1),
(14, 1, '1013', '1013', NULL, 1),
(15, 1, '1014', '1014', NULL, 1),
(16, 1, '1015', '1015', NULL, 1),
(17, 1, '1016', '1016', NULL, 1),
(18, 1, '1017', '1017', NULL, 1),
(19, 1, '1018', '1018', NULL, 1),
(20, 1, '1019', '1019', NULL, 1);

-- --------------------------------------------------------

--
-- Table structure for table `user_params`
--

CREATE TABLE IF NOT EXISTS `user_params` (
  `uid` int(11) NOT NULL auto_increment,
  `users_uid` int(11) NOT NULL,
  `name` varchar(64) NOT NULL,
  `value` varchar(128) NOT NULL,
  PRIMARY KEY  (`uid`),
  KEY `user_uid` (`users_uid`)
) ENGINE=MyISAM  DEFAULT CHARSET=latin1 AUTO_INCREMENT=39 ;

--
-- Dumping data for table `user_params`
--

INSERT INTO `user_params` (`uid`, `users_uid`, `name`, `value`) VALUES
(1, 1, 'password', '1234'),
(2, 1, 'vm-password', '1000'),
(3, 2, 'password', '1234'),
(4, 2, 'vm-password', '1002'),
(5, 3, 'password', '1234'),
(6, 3, 'vm-password', '1003'),
(7, 4, 'password', '1234'),
(8, 4, 'vm-password', '1004'),
(9, 5, 'password', '1234'),
(10, 5, 'vm-password', '1005'),
(11, 6, 'password', '1234'),
(12, 6, 'vm-password', '1006'),
(13, 7, 'password', '1234'),
(14, 7, 'vm-password', '1007'),
(15, 8, 'password', '1234'),
(16, 8, 'vm-password', '1008'),
(17, 9, 'password', '1234'),
(18, 9, 'vm-password', '1009'),
(19, 10, 'password', '1234'),
(20, 10, 'vm-password', '1010'),
(21, 11, 'password', '1234'),
(22, 11, 'vm-password', '1011'),
(23, 12, 'password', '1234'),
(24, 12, 'vm-password', '1012'),
(25, 13, 'password', '1234'),
(26, 13, 'vm-password', '1013'),
(27, 14, 'password', '1234'),
(28, 14, 'vm-password', '1014'),
(29, 15, 'password', '1234'),
(30, 15, 'vm-password', '1015'),
(31, 16, 'password', '1234'),
(32, 16, 'vm-password', '1016'),
(33, 17, 'password', '1234'),
(34, 17, 'vm-password', '1017'),
(35, 18, 'password', '1234'),
(36, 18, 'vm-password', '1018'),
(37, 19, 'password', '1234'),
(38, 19, 'vm-password', '1019');

-- --------------------------------------------------------

--
-- Table structure for table `user_variables`
--

CREATE TABLE IF NOT EXISTS `user_variables` (
  `uid` int(11) NOT NULL auto_increment,
  `users_uid` int(11) NOT NULL,
  `name` varchar(64) NOT NULL,
  `value` varchar(128) NOT NULL,
  PRIMARY KEY  (`uid`),
  KEY `user_uid` (`users_uid`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 AUTO_INCREMENT=1 ;

--
-- Dumping data for table `user_variables`
--


