-- MySQL dump 10.11                                                                    
--                                                                                     
-- Host: localhost    Database: callcard                                    
-- ------------------------------------------------------                              
-- Server version       5.0.77-community-log                                           

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
-- Current Database: `callcard`
--                                        

CREATE DATABASE /*!32312 IF NOT EXISTS*/ `callcard` /*!40100 DEFAULT CHARACTER SET utf8 */;

USE `callcard`;

--
-- Table structure for table `cards`
--                                  

DROP TABLE IF EXISTS `cards`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;                  
CREATE TABLE `cards` (                            
  `id` int(11) NOT NULL auto_increment,           
  `card_number` varchar(255) default NULL,        
  `balance` varchar(255) default NULL,            
  `rate` varchar(255) default NULL,               
  `first_name` varchar(255) default NULL,         
  `last_name` varchar(255) default NULL,          
  `email` varchar(255) default NULL,              
  `address` varchar(255) default NULL,            
  `city` varchar(255) default NULL,               
  `country` varchar(255) default NULL,            
  `phone_number` varchar(255) default NULL,       
  `created_at` datetime default NULL,             
  `updated_at` datetime default NULL,             
  PRIMARY KEY  (`id`)                             
) ENGINE=InnoDB DEFAULT CHARSET=utf8;             
SET character_set_client = @saved_cs_client;      

--
-- Dumping data for table `cards`
--                               

LOCK TABLES `cards` WRITE;
/*!40000 ALTER TABLE `cards` DISABLE KEYS */;
/*!40000 ALTER TABLE `cards` ENABLE KEYS */; 
UNLOCK TABLES;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

-- Dump completed on 2009-07-14  0:02:24
