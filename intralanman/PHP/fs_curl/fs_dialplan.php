<?php
/**
 * @package  FS_CURL
 * @subpackage FS_CURL_Dialplan
 * @todo i should probably add a condition_weight to order the conditions inside a given extension
 * fs_dialplan.php
 */
if (basename($_SERVER['PHP_SELF']) == basename(__FILE__)) {
    header('Location: index.php');
}

/**
 * @package FS_CURL
 * @subpackage FS_CURL_Dialplan
 * @license
 * @author Raymond Chandler (intralanman) <intralanman@gmail.com>
 * @version 0.1
 * Class for XML dialplan
*/
class fs_dialplan extends fs_curl {
    public function fs_dialplan() {
        $this -> fs_curl();
    }

    public function main() {
        $this -> comment($this -> request);
        $context = $this -> request['context'];
        $dp_array = $this -> get_dialplan($context);
        $this -> writeDialplan($dp_array);
        $this -> output_xml();
    }

    /**
     * This method will pull dialplan from database
     *
     * @param string $context context name for XML dialplan
     * @return array
     */
    private function get_dialplan($context) {
        $dp_array = array();
        $dpQuery = sprintf("%s %s;"
        , "SELECT * FROM dialplan WHERE context='$context' ORDER BY"
        , "global_weight, context, extension, weight"
        );
        $res = $this -> db -> query($dpQuery);
        if (MDB2::isError($res)) {
            $this -> comment($res -> getMessage());
            $this -> file_not_found();
        }
        $condition_number = 0;
        while ($row = $res -> fetchRow()) {
            $ct = $row['context'];
            $et = $row['extension'];
            $ec = $row['ext_continue'];
            $app = $row['application_name'];
            $data = $row['application_data'];
            $type = $row['type'];
            $cf = $row['condition_field'];
            $ce = $row['condition_expression'];
            $cc = empty($row['condition_continue']) ? '0' : $row['condition_continue'];
            $dp_array[$ct]["$et;$ec"]["$cf;$ce;$cc"][] = array(
            'type'=>$type,
            'application'=>$app,
            'data'=>$data
            );
        }
        return $dp_array;
    }

    /**
     * Write XML dialplan from the array returned by get_dialplan
     * @see fs_dialplan::get_dialplan
     * @param array $dpArray Multi-dimentional array from which we write the XML
     */
    private function writeDialplan($dpArray) {
        //print_r($dpArray);
        if (is_array($dpArray)) {
            $this -> xmlw -> startElement('section');
            $this -> xmlw -> writeAttribute('name', 'dialplan');
            $this -> xmlw -> writeAttribute('description', 'FreeSWITCH Dialplan');
            //$this -> comment('dpArray is an array');
            foreach ($dpArray as $context => $extensions_array) {
                //$this -> comment($context);
                //start the context
                $this -> xmlw -> startElement('context');
                $this -> xmlw -> writeAttribute('name', $context);
                if (is_array($extensions_array)) {
                    foreach ($extensions_array as $extension => $conditions) {
                        //start an extension
                        $ex_split = split(';', $extension);
                        $this -> xmlw -> startElement('extension');
                        $this -> xmlw -> writeAttribute('name', $ex_split[0]);
                        if ($ex_split[1] > 0) {
                            $this -> xmlw -> writeAttribute('continue', $ex_split[1]);
                        }
                        foreach ($conditions as $condition => $app_array) {
                            $c_split = split(';', $condition);
                            $this -> xmlw -> startElement('condition');
                            if (!empty($c_split[0])) {
                                $this -> xmlw -> writeAttribute('field', $c_split[0]);
                            }
                            if (!empty($c_split[1])) {
                                $this -> xmlw -> writeAttribute('expression', $c_split[1]);
                            }
                            //$this -> comment($c_split[2]);
                            if ($c_split[2] != '0') {
                                $this -> xmlw -> writeAttribute('continue', $c_split[2]);
                            }
                            foreach ($app_array as $app) {
                                $this -> xmlw -> startElement($app['type']);
                                $this -> xmlw -> writeAttribute('application', $app['application']);
                                if (!empty($app['data'])) {
                                    $this -> xmlw -> writeAttribute('data', $app['data']);
                                }
                                $this -> xmlw -> endElement();
                            }
                            //</condition>
                            $this -> xmlw -> endElement();
                        }
                        // </extension>
                        $this -> xmlw -> endElement();
                    }
                }
                // </context>
                $this -> xmlw -> endElement();
            }
            // </section>
            $this -> xmlw -> endElement();
        }
    }
}
?>