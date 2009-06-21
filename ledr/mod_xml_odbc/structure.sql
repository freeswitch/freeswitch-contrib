CREATE TABLE `dir_domains` (
  `id` INT NOT NULL AUTO_INCREMENT,
  `name` VARCHAR(255) NOT NULL,
  PRIMARY KEY (`id`),
  KEY `dir_domains_name` (`name`)
);

CREATE TABLE `dir_domain_params` (
  `id` INT NOT NULL AUTO_INCREMENT,
  `dir_domain_id` INT NOT NULL,
  `name` VARCHAR(255) NOT NULL,
  `value` VARCHAR(255) NOT NULL,
  PRIMARY KEY (`id`),
  FOREIGN KEY (`dir_domain_id`) REFERENCES `dir_domains` (`id`)
);

CREATE TABLE `dir_domain_variables` (
  `id` INT NOT NULL AUTO_INCREMENT,
  `dir_domain_id` INT NOT NULL,
  `name` VARCHAR(255) NOT NULL,
  `value` VARCHAR(255) NOT NULL,
  PRIMARY KEY (`id`),
  FOREIGN KEY (`dir_domain_id`) REFERENCES `dir_domains` (`id`)
);

CREATE TABLE `dir_groups` (
  `id` INT NOT NULL AUTO_INCREMENT,
  `name` VARCHAR(255) NOT NULL,
  PRIMARY KEY (`id`)
);

CREATE TABLE `dir_groups_users` (
  `dir_group_id` INT NOT NULL,
  `dir_user_id` INT NOT NULL,
  FOREIGN KEY (`dir_group_id`) REFERENCES `dir_groups` (`id`),
  FOREIGN KEY (`dir_user_id`) REFERENCES `dir_users` (`id`)
);

CREATE TABLE `dir_users` (
  `id` INT NOT NULL AUTO_INCREMENT,
  `dir_domain_id` INT NOT NULL,
  `enabled` TINYINT(1) NOT NULL,
  `username` VARCHAR(255) NOT NULL,
  `cidr` VARCHAR(255),
  `mailbox` VARCHAR(255),
  `number-alias` VARCHAR(255),
  PRIMARY KEY (`id`),
  KEY `dir_users_username` (`username`),
  KEY `dir_users_cidr` (`cidr`),
  KEY `dir_users_mailbox` (`mailbox`),
  KEY `dir_users_number-alias` (`number-alias`),
  FOREIGN KEY (`dir_domain_id`) REFERENCES `dir_domains` (`id`)
);

CREATE TABLE `dir_user_params` (
  `id` INT NOT NULL AUTO_INCREMENT,
  `dir_user_id` INT NOT NULL,
  `name` VARCHAR(255) NOT NULL,
  `value` VARCHAR(255) NOT NULL,
  PRIMARY KEY  (`id`),
  FOREIGN KEY (`dir_user_id`) REFERENCES `dir_users` (`id`)
);

CREATE TABLE `dir_user_variables` (
  `id` INT NOT NULL AUTO_INCREMENT,
  `dir_user_id` INT NOT NULL,
  `name` VARCHAR(255) NOT NULL,
  `value` VARCHAR(255) NOT NULL,
  PRIMARY KEY  (`id`),
  FOREIGN KEY (`dir_user_id`) REFERENCES `dir_users` (`id`)
);
