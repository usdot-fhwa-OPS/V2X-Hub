-- MySQL 8.0 for Linux amd64 (x86_64) and arm64 (aarch64)
--
-- Host: 127.0.0.1    Database: PORT_DRAYAGE
-- ------------------------------------------------------
-- Server version	7.6.0
-- Current Database: `PORT_DRAYAGE`
--

CREATE DATABASE /*!32312 IF NOT EXISTS*/ `PORT_DRAYAGE` /*!40100 DEFAULT CHARACTER SET latin1 */;

USE `PORT_DRAYAGE`;

--
-- Table structure for table `first_action`
--

DROP TABLE IF EXISTS `first_action`;
CREATE TABLE `first_action` (
  `cmv_id` varchar(20) NOT NULL,
  `cargo_id` varchar(20) DEFAULT NULL,
  `destination_lat` decimal(9,7) NOT NULL,
  `destination_long` decimal(9,7) NOT NULL,
  `operation` varchar(20) NOT NULL,
  `action_id` varchar(36) NOT NULL,
  `next_action` varchar(36) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

LOCK TABLES `first_action` WRITE;
INSERT INTO `first_action` VALUES ('C1T-1','NULL',-1.4,-1.4,'ENTER_STAGING_AREA','one','two');
UNLOCK TABLES;

--
-- Table structure for table `freight`
--

DROP TABLE IF EXISTS `freight`;
CREATE TABLE `freight` (
  `cmv_id` varchar(20) NOT NULL,
  `cargo_id` varchar(20) DEFAULT NULL,
  `destination_lat` decimal(9,7) NOT NULL,
  `destination_long` decimal(9,7) NOT NULL,
  `operation` varchar(20) NOT NULL,
  `action_id` varchar(36) NOT NULL,
  `next_action` varchar(36) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

LOCK TABLES `freight` WRITE;
INSERT INTO `freight` VALUES ('C1T-1','NULL',-2.4,-2.4,'EXIT_PORT','zero','one'),('C1T-1','NULL',-1.4,-1.4,'ENTER_STAGING_AREA','one','two'),('C1T-1','CARGO_A',-0.4,-0.4,'PICKUP','two','three'),('C1T-1','CARGO_A',-3.4,-1.4,'EXIT_STAGING_AREA','three','four'),('C1T-1','CARGO_A',-3.4,-3.4,'ENTER_PORT','four','five'),('C1T-1','CARGO_A',-1.4,-6.4,'DROPOFF','five','six'),('C1T-1','CARGO_B',0.6,-6.4,'PICKUP','six','seven'),('C1T-1','CARGO_B',2.6,-5.4,'PORT_CHECKPOINT','seven','eight'),('C1T-1','CARGO_B',-2.4,-2.4,'EXIT_PORT','eight','nine');
UNLOCK TABLES;