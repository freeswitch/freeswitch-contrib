<?php
/**
 * @package FS_CURL
 * @subpackage FS_CURL_Configuration
 * conference.conf.php
 */
if (basename($_SERVER['PHP_SELF']) == basename(__FILE__)) {
    header('Location: index.php');
}

/**
 * @package  FS_CURL
 * @subpackage FS_CURL_Configuration
 * @license
 * @author Raymond Chandler (intralanman) <intralanman@gmail.com>
 * @version 0.1
 * Write XML for conference.conf
*/
class conference_conf extends fs_configuration {
    /**
     * Initializer method
     * This method calls fs_configuration to initialize all of the
     * objects and variables that we want to inherit
     * @return void
    */
    public function conference_conf() {
        $this -> fs_configuration();
    }

    /**
     * Main sub-routine
     * This method will call all of the other methods necessary
     * to write out the XML for the conference.conf
     * @return void
    */
    public function main() {
        $this -> write_advertises();
        $this -> write_controls();
        $this -> write_profiles();
    }

    /**
     * Write XML for <advertises>
     * This method will write the elements and attributes for the
     * conferences that are to be advertised
     * @return void
    */
    private function write_advertises() {
        $query = "SELECT * FROM conference_advertise ORDER BY room";
        $advertises = $this -> db -> queryAll($query, null, MDB2_FETCHMODE_ASSOC);
        if (MDB2::isError($advertises)) {
            $this -> comment($query);
            $this -> comment($advertises -> getMessage());
            return ;
        }
        $advertise_count = count($advertises);
        if ($advertise_count < 1) {
            return ;
        }
        $this -> xmlw -> startElement('advertise');
        for ($i=0; $i<$advertise_count; $i++) {
            //$this -> comment_array($advertises[$i]);
            $this -> xmlw -> startElement('room');
            $this -> xmlw -> writeAttribute('name', $advertises[$i]['room']);
            $this -> xmlw -> writeAttribute('status', $advertises[$i]['status']);
            $this -> xmlw -> endElement();
        }
        $this -> xmlw -> endElement();
    }


    /**
     * Pull conference profiles from the database
     * This method will pull the conference profiles
     * from the database using the PEAR MDB2 extension
     * @return array
    */
    private function get_profiles_array() {
        $query = sprintf('%s %s;'
        , "SELECT * FROM conference_profiles"
        , "ORDER BY profile_name"
        );
        $res = $this -> db -> query($query);
        if (MDB2::isError($res)) {
            $this -> comment($query);
            $this -> comment($res -> getMessage());
            return array();
        }
        while ($row = $res -> fetchRow()) {
            $pn = $row['profile_name'];
            $profiles[$pn][] = $row;
        }
        return $profiles;
    }

    /**
     * Write XML for <profiles>
     * This method will write the XML of the array
     * from get_profiles_array
     * @return void
    */
    private function write_profiles() {
        $profiles_array = $this -> get_profiles_array();
        $profiles_count = count($profiles_array);
        if ($profiles_count < 1) {
            return ;
        }
        $this -> xmlw -> startElement('profiles');

        foreach ($profiles_array as $profile_name => $profile_data) {
            $this -> xmlw -> startElement('profile');
            $this -> xmlw -> writeAttribute('name', $profile_name);
            foreach ($profile_data as $params) {
                //$this -> comment_array($profiles_array[$i]);
                $this -> xmlw -> startElement('param');
                $this -> xmlw -> writeAttribute('name', $params['param_name']);
                $this -> xmlw -> writeAttribute('value', $params['param_value']);
                $this -> xmlw -> endElement();//</param>

            }
            $this -> xmlw -> endElement();
        }
        $this -> xmlw -> endElement();
    }

    /**
     * Pull caller-controls from database via MDB2
     * This method will pull the conference caller-controls from
     * the database using the PEAR MDB2 extension
     * @return array
    */
    private function get_controls_array() {
        $query = sprintf(
        "SELECT * FROM conference_controls ORDER BY conf_group"
        );
        $res = $this -> db -> query($query);
        if (MDB2::isError($res)) {
            $this -> comment($query);
            $this -> comment($res -> getMessage());
            return array();
        }
        while ($row = $res -> fetchRow()) {
            $cg = $row['conf_group'];
            $profiles[$cg][] = $row;
        }
        return $profiles;
    }

    /**
     * Write XML for <caller-controls>
     * This method will write the XML of the array
     * from get_controls_array
     * @return void
    */
    private function write_controls() {
        $controls_array = $this -> get_controls_array();
        $controls_count = count($controls_array);
        if (MDB2::isError($controls_array)) {
            $this -> comment($query);
            $this -> comment($controls_array -> getMessage());
            return ;
        }
        if ($controls_count < 1) {
            return ;
        }
        $this -> xmlw -> startElement('caller-controls');

        foreach ($controls_array as $group_name => $control_data) {
            $this -> xmlw -> startElement('group');
            $this -> xmlw -> writeAttribute('name', $group_name);
            foreach ($control_data as $controls) {

                //$this -> comment_array($controls_array[$i]);
                $this -> xmlw -> startElement('control');
                $this -> xmlw -> writeAttribute('action', $controls['action']);
                $this -> xmlw -> writeAttribute('digits', $controls['digits']);
                $this -> xmlw -> endElement();

            }
            $this -> xmlw -> endElement();
        }
        $this -> xmlw -> endElement();
    }
}
?>