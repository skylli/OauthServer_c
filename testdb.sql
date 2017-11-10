-- Mysql/MariaDB init script
-- Create database and user
-- CREATE DATABASE `glewlwyd`;
-- GRANT ALL PRIVILEGES ON glewlwyd.* TO 'glewlwyd'@'%' identified BY 'glewlwyd';
-- FLUSH PRIVILEGES;
-- USE `glewlwyd`;

DROP TABLE IF EXISTS `g_product`;
DROP TABLE IF EXISTS `g_resource`;
DROP TABLE IF EXISTS `g_function`;
DROP TABLE IF EXISTS `g_product_function`;


-- ----------- --
-- Data tables --
-- ----------- --
-- Code table, used to store auth code sent with response_type code and validate it with response_type authorization_code
CREATE TABLE `g_product` (
  `gp_id` INT(11) PRIMARY KEY AUTO_INCREMENT,

  `gp_name` VARCHAR(128) NOT NULL,
  `gp_manufacturers` VARCHAR(64) NOT NULL,
  `gco_devType` VARCHAR(64) default 'NULL'
);
-- Resource table, contains all registered resource server
CREATE TABLE `g_resource` (
  `gr_id` INT(11) PRIMARY KEY AUTO_INCREMENT,
  `gp_deviceId` VARCHAR(16) NOT NULL,
  `gr_name` VARCHAR(128) NOT NULL UNIQUE,
  `gr_description` VARCHAR(512) DEFAULT '',
  `gr_uri` VARCHAR(512)
);
CREATE INDEX `i_g_resource_name` ON `g_resource`(`gr_name`);
CREATE TABLE `g_function` (
  `gf_id` INT(11) PRIMARY KEY AUTO_INCREMENT,
  `gf_name` VARCHAR(128) NOT NULL,
  `gf_description` VARCHAR(128) NOT NULL
);


--  relate database
CREATE TABLE `g_product_function` (
  `gpf_id` INT(11) PRIMARY KEY AUTO_INCREMENT,
  `gp_id` INT(11) NOT NULL,
  `gf_id` INT(11) NOT NULL,
  FOREIGN KEY(`gp_id`) REFERENCES `g_product`(`gp_id`) ON DELETE CASCADE,
  FOREIGN KEY(`gf_id`) REFERENCES `g_function`(`gf_id`) ON DELETE CASCADE
);
-- Resource table, contains all registered resource server


-- Resource table, contains all registered resource server
CREATE TABLE `g_resource` (
  `gr_id` INT(11) PRIMARY KEY AUTO_INCREMENT,
  `gr_uuid` CHAR(16) NOT NULL UNIQUE,
  `gr_aeskey` CHAR(16) NOT NULL UNIQUE,
  `gr_name` VARCHAR(128) NOT NULL UNIQUE,
  `gr_description` VARCHAR(512) DEFAULT '',
  `gr_uri` VARCHAR(512)
);
-- user and resource references.
CREATE TABLE `g_user_resource` (
  `gur_id` INT(11) PRIMARY KEY AUTO_INCREMENT,
  `gu_id` INT(11) NOT NULL,
  `gr_id` INT(11) NOT NULL,
  FOREIGN KEY(`gr_id`) REFERENCES `g_resource`(`gr_id`) ON DELETE CASCADE,
  FOREIGN KEY(`gu_id`) REFERENCES `g_user`(`gu_id`) ON DELETE CASCADE
);

