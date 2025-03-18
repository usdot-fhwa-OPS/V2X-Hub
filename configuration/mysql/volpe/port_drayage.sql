-- MySQL dump 10.13  Distrib 5.7.34, for Linux (x86_64)
--
-- Host: 127.0.0.1    Database: PORT_DRAYAGE
-- ------------------------------------------------------
-- Server version	5.7.35

--
-- Current Database: `PORT_DRAYAGE`
--

CREATE DATABASE  if not exists `PORT_DRAYAGE`;

USE `PORT_DRAYAGE`;
--
-- Table structure for table `action`
--

DROP TABLE IF EXISTS `action`;
CREATE TABLE `action` (
    `action_id` INT NOT NULL,
    `prev_action_id` INT NOT NULL,
    `next_action_id` INT NOT NULL,
    `veh_id` varchar(20) NOT NULL,
    `veh_name` varchar(45) NOT NULL,
    `cargo_name` varchar(20) DEFAULT NULL,
    `cargo_uuid` varchar(45) DEFAULT NULL,
    `area_lat` decimal(9,7) NOT NULL,
    `area_long` decimal(9,7) NOT NULL,
    `area_name` varchar(45) NOT NULL,
    `area_status` varchar(20) NOT NULL,
    `area_is_notify` boolean NOT NULL,
    `created_at` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    `updated_at` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    PRIMARY KEY (`action_id`)
) 
-- ENGINE=InnoDB DEFAULT CHARSET=latin1
;

--
-- Dumping data for table `action`
--

LOCK TABLES `action` WRITE;
INSERT INTO `action` (`action_id`, `prev_action_id`, `next_action_id`, `veh_id`, `veh_name`, `cargo_name`, `cargo_uuid`, `area_lat`, `area_long`, `area_name`, `area_status`, `area_is_notify`) VALUES
(0, -1, 1, 'RED-TRUCK', 'Truck A', '', uuid(), -2.45, 0.55, 'ENTER_STAGING_AREA', 'OPEN', 0),
(1, 0, 2, 'RED-TRUCK', 'Truck A', 'Cargo X', uuid(), -3.45, -1.45, 'PICKUP', 'OPEN', 1),
(2, 1, 3, 'RED-TRUCK', 'Truck A', 'Cargo X', uuid(), -3.45, -4.45, 'EXIT_STAGING_AREA', 'OPEN', 0),
(3, 2, 4, 'RED-TRUCK', 'Truck A', 'Cargo X', uuid(), -1.45, -6.45, 'ENTER_PORT_AREA', 'OPEN', 0),
(4, 3, 5, 'RED-TRUCK', 'Truck A', 'Cargo X', uuid(), 0.55, -6.45, 'DROPOFF', 'OPEN', 1),
(5, 4, 6, 'RED-TRUCK', 'Truck A', 'Cargo X', uuid(), 1.55, -4.45, 'PICKUP', 'OPEN', 1),
(6, 5, 7, 'RED-TRUCK', 'Truck A', 'Cargo X', uuid(), 0.55, -3.45, 'PORT_CHECKPOINT', 'OPEN', 1),
(7, 6, -1, 'RED-TRUCK', 'Truck A', 'Cargo X', uuid(), -0.45, -0.45, 'EXIT_PORT_AREA', 'OPEN', 0);
UNLOCK TABLES;
