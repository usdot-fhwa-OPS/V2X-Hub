-- MySQL dump 10.13  Distrib 5.7.36, for Linux (x86_64)
--
-- Host: 127.0.0.1    Database: PORT_DRAYAGE
-- ------------------------------------------------------
-- Server version	5.7.36

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
-- Current Database: `PORT_DRAYAGE`
--

CREATE DATABASE /*!32312 IF NOT EXISTS*/ `PORT_DRAYAGE` /*!40100 DEFAULT CHARACTER SET latin1 */;

USE `PORT_DRAYAGE`;

--
-- Table structure for table `first_action`
--

DROP TABLE IF EXISTS `first_action`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `first_action` (
  `cmv_id` varchar(20) NOT NULL,
  `cargo_id` varchar(20) DEFAULT NULL,
  `destination_lat` decimal(9,7) NOT NULL,
  `destination_long` decimal(9,7) NOT NULL,
  `operation` varchar(20) NOT NULL,
  `action_id` varchar(36) NOT NULL,
  `next_action` varchar(36) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `first_action`
--

LOCK TABLES `first_action` WRITE;
/*!40000 ALTER TABLE `first_action` DISABLE KEYS */;
INSERT INTO `first_action` VALUES ('DOT-80550','CARGO_A',28.1249788,-81.8348897,'PICKUP','4bea1c45-e421-11eb-a8cc-000c29ae389d','32320c8a-e422-11eb-a8cc-000c29ae389d'),('DOT-10004','CARGO_A',28.1249788,-81.8348897,'PICKUP','66bba4a0-52c1-11ec-9105-000145098e4f','66bba4cb-52c1-11ec-9105-000145098e4f');
/*!40000 ALTER TABLE `first_action` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `freight`
--

DROP TABLE IF EXISTS `freight`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `freight` (
  `cmv_id` varchar(20) NOT NULL,
  `cargo_id` varchar(20) DEFAULT NULL,
  `destination_lat` decimal(9,7) NOT NULL,
  `destination_long` decimal(9,7) NOT NULL,
  `operation` varchar(20) NOT NULL,
  `action_id` varchar(36) NOT NULL,
  `next_action` varchar(36) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `freight`
--

LOCK TABLES `freight` WRITE;
/*!40000 ALTER TABLE `freight` DISABLE KEYS */;
INSERT INTO `freight` VALUES ('DOT-80550',NULL,28.1232195,-81.8348278,'EXIT_STAGING_AREA','32320c8a-e422-11eb-a8cc-000c29ae389d','4ace39e6-ee36-11eb-9a03-0242ac130003'),('DOT-80550','CARGO_A',28.1249788,-81.8348897,'PICKUP','4bea1c45-e421-11eb-a8cc-000c29ae389d','32320c8a-e422-11eb-a8cc-000c29ae389d'),('DOT-80550',NULL,28.1128156,-81.8314745,'ENTER_PORT','4ace39e6-ee36-11eb-9a03-0242ac130003','67eadd3a-38b4-11ec-930a-000145098e4f'),('DOT-80550','CARGO_B',28.1232336,-81.8347566,'ENTER_STAGING_AREA','fc15d52a-3c0c-11ec-b00d-000145098e4f','5ceaab82-515c-11ec-9e2c-000145098e47'),('DOT-10004','CARGO_A',28.1249788,-81.8348897,'PICKUP','66bba4a0-52c1-11ec-9105-000145098e4f','66bba4cb-52c1-11ec-9105-000145098e4f'),('DOT-10004',NULL,28.1232195,-81.8348278,'EXIT_STAGING_AREA','66bba4cb-52c1-11ec-9105-000145098e4f','84f6b797-52c2-11ec-9105-000145098e4f'),('DOT-10004',NULL,28.1128156,-81.8314745,'ENTER_PORT','84f6b797-52c2-11ec-9105-000145098e4f','c9bba171-52c2-11ec-9105-000145098e4f'),('DOT-10004','CARGO_B',28.1232336,-81.8347566,'ENTER_STAGING_AREA','20b546e3-52c6-11ec-9105-000145098e4f','53875eba-52c7-11ec-9105-000145098e4f');
/*!40000 ALTER TABLE `freight` ENABLE KEYS */;
UNLOCK TABLES;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

-- Dump completed on 2021-12-03 11:25:03
