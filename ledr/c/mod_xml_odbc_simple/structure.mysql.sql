CREATE TABLE `users` (
  `domain` VARCHAR(255) NOT NULL,
  `id` VARCHAR(255) NOT NULL,
  `mailbox` VARCHAR(255),
  `cidr` VARCHAR(255),
  `number-alias` VARCHAR(255),
  `password` VARCHAR(255),
  `dial-string` VARCHAR(255),
  `user_context` VARCHAR(255),
  PRIMARY KEY (`domain`, `id`),
  KEY `users_mailbox` (`mailbox`),
  KEY `users_cidr` (`cidr`),
  KEY `users_number-alias` (`number-alias`)
);
