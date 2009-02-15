<?php
/**
 * @package FS_CURL
 * @subpackage FS_CURL_Directory
 * fs_directory.php
*/
if (basename($_SERVER['PHP_SELF']) == basename(__FILE__)) {
    header('Location: index.php');
}

/**
 * @package FS_CURL
 * @subpackage FS_CURL_Directory
 * @author Raymond Chandler (intralanman) <intralanman@gmail.com>
 * @license
 * @author Raymond Chandler (intralanman) <intralanman@gmail.com>
 * @version 0.1
 * Class for XML directory
*/
class fs_directory extends fs_curl {
    private $users_vars;
    private $users_params;
    private $users_gateways;
    private $users_gateway_params;

    public function fs_directory() {
        $this -> fs_curl();
    }

    public function main() {
        $this -> comment($this -> request);
        $directory_array = $this -> get_directory();
        $this -> writedirectory($directory_array);
        $this -> output_xml();
    }

    /**
     * This method will pull directory from database
     * @return array
     * @todo add GROUP BY to query to make sure we don't get duplicate users
     */
    private function get_directory() {
        $directory_array = array();
        if (!array_key_exists('domain', $this -> request)) {
            $this -> comment('domain not passed');
            $this -> file_not_found();
        }
        if (GLOBAL_USERS != true) {
            $where_array[] = sprintf("domain='%s'", $this -> request['domain']);
        }
        if (array_key_exists('user', $this -> request)) {
            $where_array[] = sprintf("username='%s'", $this -> request['user']);
        }
        if (!empty($where_array)) {
            $this -> comment('where array has contents');
            if (count($where_array) > 1) {
                $where_clause = sprintf('WHERE %s', implode(' AND ', $where_array));
            } else {
                $where_clause = sprintf('WHERE %s', $where_array[0]);
            }
        } else {
            $where_clause = '';
        }
        $query = sprintf(
        "SELECT * FROM directory %s ORDER BY username"
            , $where_clause
        );
        $res = $this -> db -> queryAll($query);
        if (FS_PDO::isError($res)) {
            $this -> comment($query);
            $this -> comment($this -> db -> getMessage());
            $this -> comment($this -> db -> getMessage());
            $this -> file_not_found();
        }
        return $res;
    }

    /**
    * This method will pull the params for every user in a domain
    * @return array of users' params
    */
    private function get_users_params() {
        $query = sprintf("SELECT * FROM directory_params");
        $res = $this -> db -> queryAll($query);
        if (FS_PDO::isError($res)) {
            $this -> comment($query);
            $this -> comment($this -> db -> getMessage());
            $this -> file_not_found();
        }

        $record_count = count($res);
        for($i=0; $i<$record_count; $i++) {
            $di = $res[$i]['directory_id'];
            $pn = $res[$i]['param_name'];
            $this->users_params[$di][$pn] = $res[$i]['param_value'];
        }
    }

    /**
     * Writes XML for directory user's <params>
     * This method will pull all of the user params based on the passed user_id
     * @param integer $user_id
     * @return void
     */
    private function write_params($user_id) {
        if (is_array($this->users_params[$user_id])
            && count($this->users_params[$user_id]) > 0) {
            $this -> xmlw -> startElement('params');
            foreach($this->users_params[$user_id] as $pname => $pvalue) {
                $this -> xmlw -> startElement('param');
                $this -> xmlw -> writeAttribute('name', $pname);
                $this -> xmlw -> writeAttribute('value', $pvalue);
                $this -> xmlw -> endElement();
            }
            $this -> xmlw -> endElement();
        }
    }

    /**
     * Get all the users' variables
     */
    private function get_users_vars() {
        $query = sprintf("SELECT * FROM directory_vars;");
        $res = $this -> db -> queryAll($query);
        if (FS_PDO::isError($res)) {
            $this -> comment($this -> db -> getMessage());
            $this -> file_not_found();
        }

        $record_count = count($res);
        for($i=0; $i<$record_count; $i++) {
            $di = $res[$i]['directory_id'];
            $vn = $res[$i]['var_name'];
            $this->users_vars[$di][$vn] = $res[$i]['var_value'];
        }
    }

    /**
     * Write all the <variables> for a given user
     *
     * @param integer $user_id
     * @return void
     */
    private function write_variables($user_id) {
        if (array_key_exists($user_id, $this->users_vars)
            && is_array($this->users_vars[$user_id])) {
            $this -> xmlw -> startElement('variables');
            foreach($this->users_vars[$user_id] as $vname => $vvalue) {
                $this -> xmlw -> startElement('variable');
                $this -> xmlw -> writeAttribute('name', $vname);
                $this -> xmlw -> writeAttribute('value', $vvalue);
                $this -> xmlw -> endElement();
            }
            $this -> xmlw -> endElement();
        }
    }

    /**
     * get the users' gateways
     */
    private function get_users_gateways() {
        $query = sprintf("SELECT * FROM directory_gateways;");
        $res = $this -> db -> queryAll($query);
        if (FS_PDO::isError($res)) {
            $this -> comment($this -> db -> getMessage());
            $this -> file_not_found();
        }
        $record_count = count($res);
        for($i=0; $i<$record_count; $i++) {
            $di = $res[$i]['directory_id'];
            $this->users_gateways[$di][] = $res[$i];
        }
    }

    /**
     * Write all of the XML for the user's <gateways>
     * This method takes the id of the user from the directory table and pulls
     * all of the user's gateways. It then calls write_single_gateway for
     * each of them to write out all of the params
     * @param integer $user_id
     * @return void
     */
    private function write_gateways($user_id) {
        if (array_key_exists($user_id, $this->users_gateways)
            && is_array($this -> users_gateways[$user_id])) {
            $this -> xmlw -> startElement('gateways');
            $gateway_count = count($this -> users_gateways[$user_id]);
            for ($i=0; $i<$gateway_count; $i++) {
                $this -> xmlw -> startElement('gateway');
                $this -> xmlw -> writeAttribute(
                'name', $this -> users_gateways[$user_id][$i]['gateway_name']
                );

                $this -> write_user_gateway_params(
                    $this -> users_gateways[$user_id][$i]['id']
                );
                $this -> xmlw -> endElement();
            }
            $this -> xmlw -> endElement();
        }
    }

    /**
     * get users' gateway params
     */
    private function get_user_gateway_params() {
        $query = sprintf("SELECT * FROM directory_gateway_params;");
        $res = $this -> db -> queryAll($query);
        if (FS_PDO::isError($res)) {
            $this -> comment($this -> db -> getMessage());
            $this -> file_not_found();
        }
        $param_count = count($res);
        for($i=0; $i<$param_count; $i++) {
            $dgwid = $res[$i]['id'];
            $pname = $res[$i]['param_name'];
            $pvalue = $res[$i]['param_value'];
            $this->users_gateway_params[$dgwid][$pname] = $pvalue;
        }
    }

    /**
     * Write out the <params> XML for each user-specific gateway
     *
     * @param integer $d_gw_id id from directory_gateways
     * @return void
     */
    private function write_user_gateway_params($d_gw_id) {
        if (is_array($this->users_gateway_params[$d_gw_id])
            && count($this->users_gateway_params[$d_gw_id]) > 0) {
            foreach($this->users_gateway_params[$d_gw_id] as $pname => $pvalue) {
                $this -> xmlw -> startElement('param');
                $this -> xmlw -> writeAttribute('name', $pname);
                $this -> xmlw -> writeAttribute('value', $pvalue);
                $this -> xmlw -> endElement();
            }
        }
    }

    /**
     * This method will write out XML for global directory params
     *
     */
    function write_global_params() {
        $query = sprintf('%s %s %s;',
        'SELECT * FROM directory_global_params dgp',
        'JOIN directory_domains dd ON dd.id=dgp.domain_id',
        "WHERE dd.domain_name='" . $this -> request['domain'] . "'"
        );
        $res = $this -> db -> queryAll($query);
        if (FS_PDO::isError($res)) {
            $this -> comment($query);
            $error_msg = sprintf("Error while selecting global params - %s"
                , $this -> db -> getMessage()
            );
            trigger_error($error_msg);
        }
        $param_count = count($res);
        $this -> xmlw -> startElement('params');
        for ($i=0; $i<$param_count; $i++) {
            $this -> xmlw -> startElement('param');
            $this -> xmlw -> writeAttribute('name', $res[$i]['param_name']);
            $this -> xmlw -> writeAttribute('value', $res[$i]['param_value']);
            $this -> xmlw -> endElement();
        }
        $this -> xmlw -> endElement();
    }

    /**
     * This method will write out XML for global directory variables
     *
     */
    function write_global_vars() {
        $query = sprintf('%s %s %s;',
        'SELECT * FROM directory_global_vars dgv',
        'JOIN directory_domains dd ON dd.id=dgv.domain_id',
        "WHERE dd.domain_name='" . $this -> request['domain'] . "'"
        );
        $res = $this -> db -> queryAll($query);
        if (FS_PDO::isError($res)) {
            $this -> comment($query);
            $error_msg = sprintf("Error while selecting global vars - %s"
                , $this -> db -> getMessage()
            );
            trigger_error($error_msg);
        }
        $param_count = count($res);
        $this -> xmlw -> startElement('variables');
        for ($i=0; $i<$param_count; $i++) {
            $this -> xmlw -> startElement('variable');
            $this -> xmlw -> writeAttribute('name', $res[$i]['var_name']);
            $this -> xmlw -> writeAttribute('value', $res[$i]['var_value']);
            $this -> xmlw -> endElement();
        }
        $this -> xmlw -> endElement();
    }

    /**
     * Write XML directory from the array returned by get_directory
     * @see fs_directory::get_directory
     * @param array $directory Multi-dimentional array from which we write the XML
     * @return void
     */
    private function writedirectory($directory) {
        if (empty($directory)) {
            $this -> comment('no users found');
            $this -> file_not_found();
        }
        $directory_count = count($directory);

        $this -> get_users_params();
        $this -> get_users_vars();
        $this -> get_users_gateways();
        $this -> get_user_gateway_params();

        $this -> xmlw -> startElement('section');
        $this -> xmlw -> writeAttribute('name', 'directory');
        $this -> xmlw -> writeAttribute('description', 'FreeSWITCH Directory');
        $this -> xmlw -> startElement('domain');
        $this -> xmlw -> writeAttribute('name', $this -> request['domain']);
        $this -> write_global_params();
        $this -> write_global_vars();

        for ($i=0; $i<$directory_count; $i++) {
            $username = $directory[$i]['username'];
            $mailbox = empty($directory[$i]['mailbox']) ? $username : $directory[$i]['mailbox'];
            $this -> xmlw -> startElement('user');
            $this -> xmlw -> writeAttribute('id', $username);
            $this -> xmlw -> writeAttribute('mailbox', $mailbox);
            $this -> write_params($directory[$i]['id']);
            $this -> write_variables($directory[$i]['id']);
            $this -> write_gateways($directory[$i]['id']);
            $this -> xmlw -> endElement();
        }
        $this -> xmlw -> endElement();
        $this -> xmlw -> endElement();
    }
}

