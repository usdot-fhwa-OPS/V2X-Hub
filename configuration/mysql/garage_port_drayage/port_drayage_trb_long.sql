-- MySQL 8.0 for Linux amd64 (x86_64) and arm64 (aarch64)
--
-- Host: 127.0.0.1    Database: PORT_DRAYAGE
-- ------------------------------------------------------
-- Server version       7.6.0
-- Current Database: `PORT_DRAYAGE`
--
-- Map: TRB Demo (https://github.com/usdot-fhwa-stol/cda1tenth-bringup/blob/develop/maps/trb_demo.pgm)
-- Route Graph: TRB Demo (https://github.com/usdot-fhwa-stol/cda1tenth-bringup/blob/develop/graphs/trb_demo.geojson)

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
INSERT INTO `first_action` VALUES ('RED-TRUCK','NULL',1.3,0.0,'ENTER_STAGING_AREA','one','two');
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
INSERT INTO `freight` VALUES ('RED-TRUCK','NULL',1.3,0.0,'ENTER_STAGING_AREA','one','two'),('RED-TRUCK','CARGO_A',-0.2,1.4,'PICKUP','two','three'),('RED-TRUCK','NULL',1.3,0.0,'ENTER_PORT','three','four'),('RED-TRUCK','NULL',-0.2,1.4,'DROPOFF','four','five');
UNLOCK TABLES;
