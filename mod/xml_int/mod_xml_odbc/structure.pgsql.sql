-- directory domains

CREATE TABLE dir_domains (
  id SERIAL NOT NULL,
  name VARCHAR(255) NOT NULL,
  CONSTRAINT dir_domains_id PRIMARY KEY (id)
);

CREATE UNIQUE INDEX dir_domains_name_index
  ON dir_domains (name);


CREATE TABLE dir_domain_params (
  id SERIAL NOT NULL,
  dir_domain_id INTEGER NOT NULL REFERENCES dir_domains(id),
  name VARCHAR(255) NOT NULL,
  value VARCHAR(255) NOT NULL,
  CONSTRAINT dir_domain_params_id PRIMARY KEY (id)
);

CREATE UNIQUE INDEX dir_domain_params_dir_domain_id_name_index
  ON dir_domain_params (dir_domain_id, name);


CREATE TABLE dir_domain_variables (
  id SERIAL NOT NULL,
  dir_domain_id INTEGER NOT NULL REFERENCES dir_domains(id),
  name VARCHAR(255) NOT NULL,
  value VARCHAR(255) NOT NULL,
  CONSTRAINT dir_domain_variables_id PRIMARY KEY (id)
);

CREATE UNIQUE INDEX dir_domain_variables_dir_domain_id_name_index
  ON dir_domain_variables (dir_domain_id, name);



-- directory users

CREATE TABLE dir_users (
  id SERIAL NOT NULL,
  dir_domain_id INTEGER NOT NULL REFERENCES dir_domains(id),
  username VARCHAR(255) NOT NULL,
  enabled BOOLEAN NOT NULL DEFAULT 'true',
  cidr VARCHAR(255),
  mailbox VARCHAR(255),
  "number-alias" VARCHAR(255),
  CONSTRAINT dir_users_id PRIMARY KEY (id)
);

CREATE UNIQUE INDEX dir_users_dir_domain_id_username_index
  ON dir_users (dir_domain_id, username);


CREATE TABLE dir_user_params (
  id SERIAL NOT NULL,
  dir_user_id INTEGER NOT NULL REFERENCES dir_users(id),
  name VARCHAR(255) NOT NULL,
  value VARCHAR(255) NOT NULL,
  CONSTRAINT dir_user_params_id PRIMARY KEY (id)
);

CREATE UNIQUE INDEX dir_user_params_dir_user_id_name_index
  ON dir_user_params (dir_user_id, name);


CREATE TABLE dir_user_variables (
  id SERIAL NOT NULL,
  dir_user_id INTEGER NOT NULL,
  name VARCHAR(255) NOT NULL,
  value VARCHAR(255) NOT NULL,
  CONSTRAINT dir_user_variables_id PRIMARY KEY (id)
);

CREATE UNIQUE INDEX dir_user_variables_dir_user_id_name_index
  ON dir_user_variables (dir_user_id, name);



-- directory groups

CREATE TABLE dir_groups (
  id SERIAL NOT NULL,
  name VARCHAR(255) NOT NULL,
  CONSTRAINT dir_groups_id PRIMARY KEY (id)
);

CREATE UNIQUE INDEX dir_groups_name_index
  ON dir_groups (name);


CREATE TABLE dir_groups_users (
  dir_group_id INTEGER NOT NULL REFERENCES dir_groups(id),
  dir_user_id INTEGER NOT NULL REFERENCES dir_users(id)
);

CREATE UNIQUE INDEX dir_groups_users_index
  ON dir_groups_users (dir_group_id, dir_user_id);

