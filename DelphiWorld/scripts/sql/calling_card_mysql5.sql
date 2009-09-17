-- phpMyAdmin SQL Dump
-- version 3.1.3.1
-- http://www.phpmyadmin.net
--
-- Host: localhost:3306

-- Generation Time: Sep 17, 2009 at 07:12 PM
-- Server version: 5.1.34
-- PHP Version: 5.2.9-2

SET SQL_MODE="NO_AUTO_VALUE_ON_ZERO";


/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8 */;

--
-- Database: `mss`
--

-- --------------------------------------------------------

--
-- Table structure for table `tbl_caller_prepaid`
--

CREATE TABLE `tbl_caller_prepaid` (
  `cSubscriber` char(60) NOT NULL,
  `cRouteNbr` char(60) NOT NULL DEFAULT '',
  `nRateId` int(11) DEFAULT NULL,
  PRIMARY KEY (`cSubscriber`,`cRouteNbr`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Dumping data for table `tbl_caller_prepaid`
--


-- --------------------------------------------------------

--
-- Table structure for table `tbl_callingcard_balance`
--

CREATE TABLE `tbl_callingcard_balance` (
  `cCard` char(32) NOT NULL,
  `cPassword` char(32) DEFAULT NULL,
  `nBalance` int(11) DEFAULT NULL,
  PRIMARY KEY (`cCard`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Dumping data for table `tbl_callingcard_balance`
--

INSERT INTO `tbl_callingcard_balance` VALUES('1000000000', '1000000000', 50000);

-- --------------------------------------------------------

--
-- Table structure for table `tbl_callingcard_cdr`
--

CREATE TABLE `tbl_callingcard_cdr` (
  `cCard` char(32) DEFAULT NULL,
  `cCalled` char(32) DEFAULT NULL,
  `StartTime` datetime DEFAULT NULL,
  `ConnTime` datetime DEFAULT NULL,
  `ReleaseTime` datetime DEFAULT NULL,
  `nDuration` int(11) DEFAULT NULL,
  `nFee` int(11) DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Dumping data for table `tbl_callingcard_cdr`
--


-- --------------------------------------------------------

--
-- Table structure for table `tbl_callingcard_fee_matrix`
--

CREATE TABLE `tbl_callingcard_fee_matrix` (
  `cRouteNbr` char(32) NOT NULL,
  `nRateId` int(11) DEFAULT NULL,
  PRIMARY KEY (`cRouteNbr`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Dumping data for table `tbl_callingcard_fee_matrix`
--

INSERT INTO `tbl_callingcard_fee_matrix` VALUES('0', 1);

-- --------------------------------------------------------

--
-- Table structure for table `tbl_cdr`
--

CREATE TABLE `tbl_cdr` (
  `nDirection` int(11) DEFAULT NULL,
  `cCaller` char(60) DEFAULT NULL,
  `cCalled` char(60) DEFAULT NULL,
  `nSubType` int(11) DEFAULT NULL,
  `tStartTime` datetime DEFAULT NULL,
  `tConnectTime` datetime DEFAULT NULL,
  `tReleaseTime` datetime DEFAULT NULL,
  `nDuration` int(11) DEFAULT NULL,
  `nFee` int(11) DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Dumping data for table `tbl_cdr`
--


-- --------------------------------------------------------

--
-- Table structure for table `tbl_prepaid_rate`
--

CREATE TABLE `tbl_prepaid_rate` (
  `rate_id` int(11) NOT NULL,
  `tariff_interval` int(11) DEFAULT NULL,
  `tariff` int(11) DEFAULT NULL,
  PRIMARY KEY (`rate_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Dumping data for table `tbl_prepaid_rate`
--

INSERT INTO `tbl_prepaid_rate` VALUES(1, 60, 1);

-- --------------------------------------------------------

--
-- Table structure for table `tbl_sub_balance`
--

CREATE TABLE `tbl_sub_balance` (
  `cSubName` char(60) NOT NULL,
  `nBalance` int(11) DEFAULT NULL,
  PRIMARY KEY (`cSubName`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Dumping data for table `tbl_sub_balance`
--


-- --------------------------------------------------------

--
-- Table structure for table `tbl_ver`
--

CREATE TABLE `tbl_ver` (
  `table_id` int(11) NOT NULL,
  `version` int(11) DEFAULT NULL,
  `description` char(80) DEFAULT NULL,
  PRIMARY KEY (`table_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Dumping data for table `tbl_ver`
--

INSERT INTO `tbl_ver` VALUES(0, 1, 'tbl_ver');
INSERT INTO `tbl_ver` VALUES(1, 1, 'tbl_prepaid_rate');
INSERT INTO `tbl_ver` VALUES(4, 1, 'tbl_sub_balance');
INSERT INTO `tbl_ver` VALUES(2, 2, 'tbl_caller_prepaid');
INSERT INTO `tbl_ver` VALUES(3, 1, 'tbl_cdr');
INSERT INTO `tbl_ver` VALUES(20, 1, 'tbl_callingcard_balance');
INSERT INTO `tbl_ver` VALUES(22, 1, 'tbl_callingcard_fee_matrix');
INSERT INTO `tbl_ver` VALUES(21, 1, 'tbl_callingcard_cdr');

/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
