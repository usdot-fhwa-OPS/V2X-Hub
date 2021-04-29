-- MySQL dump 10.13  Distrib 5.7.33, for Linux (x86_64)
--
-- Host: 127.0.0.1    Database: PORT_DRAYAGE
-- ------------------------------------------------------
-- Server version	5.7.33

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
-- Table structure for table `freight`
--

DROP TABLE IF EXISTS `freight`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `freight` (
  `cmv_id` int(11) DEFAULT NULL,
  `cargo_id` int(11) DEFAULT NULL,
  `cargo` tinyint(1) DEFAULT NULL,
  `location` point DEFAULT NULL,
  `destination` point DEFAULT NULL,
  `operation` varchar(20) DEFAULT NULL,
  `action_id` int(11) DEFAULT NULL,
  `next_action` int(11) DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `freight`
--

LOCK TABLES `freight` WRITE;
/*!40000 ALTER TABLE `freight` DISABLE KEYS */;
INSERT INTO `freight` VALUES (1,2,0,_binary '\0\0\0\0\0\0\0\0\0\0\0\0\0ð?\0\0\0\0\0\0ð?',_binary '\0\0\0\0\0\0\0\Í\Ì\Ì\Ì\Ì\Ì\0@ffffff@','MOVING_TO_LOADING_A',32,33),(2,1,1,_binary '\0\0\0\0\0\0\0š™™™™™E@3333337À',_binary '\0\0\0\0\0\0\0š™™™™™J@š™™™™™F@','LOADING_CARGO',12,13);
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

-- Dump completed on 2021-04-28 16:35:03
