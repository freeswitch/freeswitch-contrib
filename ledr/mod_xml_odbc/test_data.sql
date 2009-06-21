INSERT INTO `dir_domains` (`id`, `name`) VALUES (1, '172.31.0.76');
INSERT INTO `dir_domain_params` (`id`, `dir_domain_id`, `name`, `value`) VALUES (1, 1, 'dom_param_name', 'dom_param_value');
INSERT INTO `dir_domain_variables` (`id`, `dir_domain_id`, `name`, `value`) VALUES (1, 1, 'dom_variable_name', 'dom_variable_value');
INSERT INTO `dir_groups` (`id`, `name`) VALUES (1, 'prepay'), (2, 'annoying_customers');
INSERT INTO `dir_groups_users` (`dir_group_id`, `dir_user_id`) VALUES (1, 1), (2, 1);
INSERT INTO `dir_users` (`id`, `dir_domain_id`, `username`, `cidr`, `mailbox`, `number-alias`) VALUES (1, 1, 'someuser', NULL, 'leon@toyos.nl', '0031320227470');
INSERT INTO `dir_user_params` (`id`, `dir_user_id`, `name`, `value`) VALUES (1, 1, 'password', 'topsecret'), (2, 1, 'vm-password', '0000');
INSERT INTO `dir_user_variables` (`id`, `dir_user_id`, `name`, `value`) VALUES (1, 1, 'accountcode', '1234'), (2, 1, 'user_context', 'default'), (3, 1, 'vm_extension', '1234');
