<?php
/**
 * @package FS_CURL
 * @license
 * @author Raymond Chandler (intralanman) <intralanman@gmail.com>
 * @version 0.1
 */

if (basename($_SERVER['PHP_SELF']) == basename(__FILE__)) {
    header('Location: index.php');
}

/**
 * Defines the default dsn for the MDB2 PEAR class
 */
define('DEFAULT_DSN', 'mysql://freeswitch:Fr33Sw1tch@localhost/freeswitch');
/**
 * Generic return success
 */
define('FS_CURL_SUCCESS', 0);
/**
 * Generic return warning
 */
define('FS_CURL_WARNING', 1);
/**
 * Generic return critical
 */
define('FS_CURL_CRITICAL', 2);

/**
 * determines how the error handler handles warnings
 */
define('RETURN_ON_WARN', true);

/**
 * Determines whether or not users should be domain specific
 * If GLOBAL_USERS is true, user info will be returned for whatever
 * domain is passed.....
 * NOTE: using a1 hashes will NOT work with this setting
 */
define('GLOBAL_USERS', true);



//define('', '');
?>