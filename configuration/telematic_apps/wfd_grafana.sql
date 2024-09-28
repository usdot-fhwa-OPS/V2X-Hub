-- MySQL dump 10.13  Distrib 8.0.39, for Linux (x86_64)
--
-- Host: grafana-db-instance-1.ckavqbffspu1.us-east-1.rds.amazonaws.com    Database: wfd_grafana
-- ------------------------------------------------------
-- Server version	5.7.12

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!50503 SET NAMES utf8mb4 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;

--
-- Current Database: `wfd_grafana`
--

CREATE DATABASE /*!32312 IF NOT EXISTS*/ `wfd_grafana` /*!40100 DEFAULT CHARACTER SET latin1 */;

USE `wfd_grafana`;
--
-- Table structure for table `default_event_topics`
--

DROP TABLE IF EXISTS `default_event_topics`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `default_event_topics` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `topic_names` text NOT NULL,
  `unit_identifier` varchar(255) NOT NULL DEFAULT '',
  `event_id` int(11) DEFAULT NULL,
  `created_at` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `created_by` int(11) NOT NULL DEFAULT '0',
  `updated_at` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `updated_by` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`),
  KEY `event_id` (`event_id`),
  CONSTRAINT `default_event_topics_ibfk_1` FOREIGN KEY (`event_id`) REFERENCES `events` (`id`) ON DELETE SET NULL ON UPDATE CASCADE
) ENGINE=InnoDB AUTO_INCREMENT=44 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `event_dashboard`
--

DROP TABLE IF EXISTS `event_dashboard`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `event_dashboard` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `event_id` int(11) NOT NULL DEFAULT '0',
  `dashboard_id` int(11) NOT NULL DEFAULT '0',
  `created` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `updated` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`id`),
  UNIQUE KEY `event_dashboard_dashboard_id_event_id` (`dashboard_id`,`event_id`)
) ENGINE=InnoDB AUTO_INCREMENT=48 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `event_dashboard`
--

LOCK TABLES `event_dashboard` WRITE;
/*!40000 ALTER TABLE `event_dashboard` DISABLE KEYS */;
INSERT INTO `event_dashboard` VALUES (47,66,91,'2024-09-24 13:35:55','2024-09-24 13:35:55');
/*!40000 ALTER TABLE `event_dashboard` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `event_units`
--

DROP TABLE IF EXISTS `event_units`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `event_units` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `event_id` int(11) NOT NULL,
  `unit_id` int(11) NOT NULL,
  `start_time` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `end_time` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `created_at` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `created_by` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`),
  UNIQUE KEY `event_units_unitId_eventId_unique` (`event_id`,`unit_id`),
  KEY `unit_id` (`unit_id`),
  CONSTRAINT `event_units_ibfk_1` FOREIGN KEY (`event_id`) REFERENCES `events` (`id`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `event_units_ibfk_2` FOREIGN KEY (`unit_id`) REFERENCES `units` (`id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB AUTO_INCREMENT=199 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `event_units`
--

LOCK TABLES `event_units` WRITE;
/*!40000 ALTER TABLE `event_units` DISABLE KEYS */;
INSERT INTO `event_units` VALUES (198,66,37,'2024-09-24 13:32:24','2025-09-29 13:32:24','2024-09-24 13:32:32',0);
/*!40000 ALTER TABLE `event_units` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `events`
--

DROP TABLE IF EXISTS `events`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `events` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `name` varchar(255) NOT NULL DEFAULT '',
  `description` varchar(255) NOT NULL DEFAULT '',
  `status` varchar(255) NOT NULL DEFAULT '',
  `location_id` int(11) DEFAULT NULL,
  `testing_type_id` int(11) DEFAULT NULL,
  `start_at` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `end_at` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `created_at` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `created_by` int(11) NOT NULL DEFAULT '0',
  `updated_at` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `updated_by` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`),
  KEY `location_id` (`location_id`),
  KEY `testing_type_id` (`testing_type_id`),
  CONSTRAINT `events_ibfk_1` FOREIGN KEY (`location_id`) REFERENCES `locations` (`id`) ON DELETE SET NULL ON UPDATE CASCADE,
  CONSTRAINT `events_ibfk_2` FOREIGN KEY (`testing_type_id`) REFERENCES `testing_types` (`id`) ON DELETE SET NULL ON UPDATE CASCADE
) ENGINE=InnoDB AUTO_INCREMENT=67 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `events`
--

LOCK TABLES `events` WRITE;
/*!40000 ALTER TABLE `events` DISABLE KEYS */;
INSERT INTO `events` VALUES (66,'Final_demo','This is an event for final demo purpose','',9,8,'2024-09-24 13:30:14','2025-09-30 13:30:13','2024-09-24 13:30:56',1,'2024-09-24 13:30:56',1);
/*!40000 ALTER TABLE `events` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `file`
--

DROP TABLE IF EXISTS `file`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `file` (
  `path` varchar(1024) COLLATE utf8mb4_unicode_ci NOT NULL,
  `path_hash` varchar(64) COLLATE utf8mb4_unicode_ci NOT NULL,
  `parent_folder_path_hash` varchar(64) COLLATE utf8mb4_unicode_ci NOT NULL,
  `contents` mediumblob,
  `etag` varchar(32) COLLATE utf8mb4_unicode_ci NOT NULL,
  `cache_control` varchar(128) COLLATE utf8mb4_unicode_ci NOT NULL,
  `content_disposition` varchar(128) COLLATE utf8mb4_unicode_ci NOT NULL,
  `updated` datetime NOT NULL,
  `created` datetime NOT NULL,
  `size` bigint(20) NOT NULL,
  `mime_type` varchar(255) COLLATE utf8mb4_unicode_ci NOT NULL,
  UNIQUE KEY `UQE_file_path_hash` (`path_hash`),
  KEY `IDX_file_parent_folder_path_hash` (`parent_folder_path_hash`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

-- --
-- -- Table structure for table `file_infos`
-- --

-- DROP TABLE IF EXISTS `file_infos`;
-- /*!40101 SET @saved_cs_client     = @@character_set_client */;
-- /*!50503 SET character_set_client = utf8mb4 */;
-- CREATE TABLE `file_infos` (
--   `id` bigint(20) NOT NULL AUTO_INCREMENT,
--   `content_location` varchar(255) NOT NULL DEFAULT '',
--   `original_filename` varchar(255) NOT NULL DEFAULT '',
--   `process_status` varchar(255) DEFAULT NULL,
--   `process_error_msg` text,
--   `size` bigint(20) unsigned DEFAULT NULL,
--   `upload_status` varchar(255) DEFAULT NULL,
--   `upload_error_msg` text,
--   `description` text,
--   `created_at` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
--   `created_by` bigint(20) NOT NULL,
--   `user_id` bigint(20) DEFAULT NULL,
--   `updated_at` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
--   `updated_by` bigint(20) NOT NULL,
--   PRIMARY KEY (`id`),
--   UNIQUE KEY `original_filename` (`original_filename`),
--   KEY `user_id` (`user_id`)
--   ) ENGINE=InnoDB AUTO_INCREMENT=351 DEFAULT CHARSET=latin1;
-- /*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `locations`
--

DROP TABLE IF EXISTS `locations`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `locations` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `facility_name` varchar(255) NOT NULL DEFAULT '',
  `city` varchar(255) NOT NULL DEFAULT '',
  `state_code` varchar(255) NOT NULL DEFAULT '',
  `zip_code` varchar(255) NOT NULL DEFAULT '',
  `created_at` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `created_by` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=12 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `locations`
--

LOCK TABLES `locations` WRITE;
/*!40000 ALTER TABLE `locations` DISABLE KEYS */;
INSERT INTO `locations` VALUES (1,'Local integration testing','NA','VA','20121','2022-10-24 17:07:52',1),(2,'Local','Test','GA','20212','2022-10-27 17:07:10',1),(3,'Local_VM_2','City2','VA','21212','2022-11-01 21:08:14',1),(4,'Local_VM testing','Testing','VA','20120','2022-11-01 21:10:36',1),(5,'Test Facility 11/01','Springfield','VA','20102','2022-11-01 21:45:10',1),(6,'test facility 6','city 6','VA','20121','2022-11-01 21:50:16',1),(7,'TFHRC','Mclean','VA','22101','2022-12-05 17:07:41',1),(8,'Demo Facility','Mclean','VA','22012','2023-01-31 19:29:19',1),(9,'test','test','GA','222222','2023-04-20 15:57:42',1),(10,'Gaylord Resort','Grapevine','TX','00000','2023-04-20 15:58:45',1),(11,'TFHRC Copy','McLean','VA','22101','2023-11-21 02:45:47',1);
/*!40000 ALTER TABLE `locations` ENABLE KEYS */;
UNLOCK TABLES;
--
-- Table structure for table `states`
--

DROP TABLE IF EXISTS `states`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `states` (
  `name` varchar(255) NOT NULL,
  `code` varchar(255) NOT NULL DEFAULT '',
  PRIMARY KEY (`name`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `states`
--

LOCK TABLES `states` WRITE;
/*!40000 ALTER TABLE `states` DISABLE KEYS */;
INSERT INTO `states` VALUES ('Alabama','AL'),('Alaska','AK'),('American Samoa','AS'),('Arizona','AZ'),('Arkansas','AR'),('California','CA'),('Colorado','CO'),('Connecticut','CT'),('Delaware','DE'),('District of Columbia','DC'),('Federated States of Micronesia','FM'),('Florida','FL'),('Georgia','GA'),('Guam','GU'),('Hawaii','HI'),('Idaho','ID'),('Illinois','IL'),('Indiana','IN'),('Iowa','IA'),('Kansas','KS'),('Kentucky','KY'),('Louisiana','LA'),('Maine','ME'),('Marshall Islands','MH'),('Maryland','MD'),('Massachusetts','MA'),('Michigan','MI'),('Minnesota','MN'),('Mississippi','MS'),('Missouri','MO'),('Montana','MT'),('Nebraska','NE'),('Nevada','NV'),('New Hampshire','NH'),('New Jersey','NJ'),('New Mexico','NM'),('New York','NY'),('North Carolina','NC'),('North Dakota','ND'),('Northern Mariana Islands','MP'),('Ohio','OH'),('Oklahoma','OK'),('Oregon','OR'),('Palau','PW'),('Pennsylvania','PA'),('Puerto Rico','PR'),('Rhode Island','RI'),('South Carolina','SC'),('South Dakota','SD'),('Tennessee','TN'),('Texas','TX'),('Utah','UT'),('Vermont','VT'),('Virgin Islands','VI'),('Virginia','VA'),('Washington','WA'),('West Virginia','WV'),('Wisconsin','WI'),('Wyoming','WY');
/*!40000 ALTER TABLE `states` ENABLE KEYS */;
UNLOCK TABLES;


--
-- Table structure for table `testing_types`
--

DROP TABLE IF EXISTS `testing_types`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `testing_types` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `name` varchar(255) NOT NULL DEFAULT '',
  `created_at` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `created_by` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=58 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `testing_types`
--

LOCK TABLES `testing_types` WRITE;
/*!40000 ALTER TABLE `testing_types` DISABLE KEYS */;
INSERT INTO `testing_types` VALUES (1,'Verification','2022-10-24 17:07:10',0),(2,'Validation','2022-10-24 17:07:10',0),(3,'Integration','2022-10-24 17:07:10',0),(4,'Verification','2024-04-02 15:35:16',0),(5,'Validation','2024-04-02 15:35:16',0),(6,'Integration','2024-04-02 15:35:16',0),(7,'Verification','2024-04-02 15:45:37',0),(8,'Validation','2024-04-02 15:45:37',0),(9,'Integration','2024-04-02 15:45:37',0);
/*!40000 ALTER TABLE `testing_types` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `units`
--

DROP TABLE IF EXISTS `units`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `units` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `unit_name` varchar(255) NOT NULL DEFAULT '',
  `unit_identifier` varchar(255) NOT NULL DEFAULT '',
  `unit_type` varchar(255) NOT NULL DEFAULT '',
  `created_at` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `created_by` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=42 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `units`
--

LOCK TABLES `units` WRITE;
/*!40000 ALTER TABLE `units` DISABLE KEYS */;
INSERT INTO `units` VALUES (37,'V2xHub Telematic Plugin','rsu_1234','V2xHub','2024-05-01 19:45:47',1),(38,'Simulation_vehicle','SimVehicle1','Platform','2024-07-16 18:30:57',1),(39,'Fusion','DOT-45254','Platform','2024-07-24 19:39:49',1);
/*!40000 ALTER TABLE `units` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `user_topic_request`
--

DROP TABLE IF EXISTS `user_topic_request`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `user_topic_request` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `topic_names` text NOT NULL,
  `unit_identifier` varchar(255) NOT NULL DEFAULT '',
  `event_id` int(11) NOT NULL DEFAULT '0',
  `created_at` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `created_by` int(11) NOT NULL DEFAULT '0',
  `updated_at` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `updated_by` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=3003 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;
