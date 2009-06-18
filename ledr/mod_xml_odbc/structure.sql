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
  PRIMARY KEY (`id`),
  FOREIGN KEY (`dir_domain_id`) REFERENCES `dir_domains` (`id`)
);

CREATE TABLE `dir_user_attrs` (
  `id` INT  NOT NULL AUTO_INCREMENT,
  `dir_user_id` INT NOT NULL,
  `name` VARCHAR(255) NOT NULL,
  `value` VARCHAR(255) NOT NULL,
  PRIMARY KEY  (`id`),
  FOREIGN KEY (`dir_user_id`) REFERENCES `dir_users` (`id`),
  KEY `dir_user_attrs_name` (`name`),
  KEY `dir_user_attrs_value` (`value`)
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
