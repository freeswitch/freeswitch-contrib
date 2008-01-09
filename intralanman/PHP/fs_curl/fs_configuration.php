<?php
/**
 * @package  FS_CURL
 * @subpackage FS_CURL_Configuration
 * fs_configuration.php
 */
if (basename($_SERVER['PHP_SELF']) == basename(__FILE__)) {
    header('Location: index.php');
}

/**
 * @package FS_CURL
 * @subpackage FS_CURL_Configuration
 * @license
 * @author Raymond Chandler (intralanman) <intralanman@gmail.com>
 * @version 0.1
 * Class for all module configurations
 * @return object
*/
class fs_configuration extends fs_curl {
    /**
     * Class Instantiation
     * This method will instantiate all other objects upon which it relies.
     * @return void
    */
    function fs_configuration() {
        $this -> fs_curl();
        $mod_name = sprintf('mod_%s', str_replace('.conf', '', $this -> request['key_value']));
        $this -> comment("module name is $mod_name");
        if (!$this -> is_mod_enabled($mod_name)) {
            $this -> comment('module not enabled');
            $this -> file_not_found();
        }
        $this -> xmlw -> startElement('section');
        $this -> xmlw -> writeAttribute('name', 'configuration');
        $this -> xmlw -> writeAttribute(
        'description', 'FreeSWITCH Configuration'
        );
    }

    /**
     * Enabled module checker
     * This method will check if a module is enabled before
     * returning the XML for the module. If the module's not
     * enabled, the file_not_found method will be called.
     * @param string $mod_name name of module to check
     * @return bool
    */
    function is_mod_enabled($mod_name) {
        if ($mod_name == 'mod_post_load_modules') {
            return true;
        }
        $query = sprintf('%s %s'
        , "SELECT * FROM post_load_modules_conf"
        , "WHERE module_name='$mod_name' AND load_module=1"
        );
        $res = $this -> db -> query($query);
        if (MDB2::isError($res)) {
            $this -> comment($query);
            $this -> comment($res -> getMessage());
            return true; //default allow policy
            return false; //comment previous line to default deny
            $this -> file_not_found();
        } elseif ($res -> numRows() == 1) {
            return true;
        } else {
            return false;
        }
    }
}

?>