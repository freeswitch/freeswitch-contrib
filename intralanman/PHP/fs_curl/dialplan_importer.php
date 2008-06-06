<?php
/**
 * @package FS_CURL
 * @subpackage FS_CURL_Dialplan
 * @license
 * @author Raymond Chandler (intralanman) <intralanman@gmail.com>
 * @version 0.1
*/

/**
 * include MDB2 PEAR Extension
 */
require_once('MDB2.php');
/**
 * require global definitions for FS_CURL
 */
require_once('global_defines.php');

/**
 * Output the upload form
 * echo out the HTML for the upload form.
 * @return null
*/
function upload_form() {
    echo '<html>';
    echo '<h1>Select A File To Import</h1>';
    echo '<form method="post" action="' . $_SERVER['PHP_SELF'] . '" enctype="multipart/form-data">';
    echo '<input type="file" name="file">';
    echo '<input type="submit" name="confirmed">';
    echo '</form>';
    echo '</html>';
}

/**
 * Perform Insert Query
 * take MDB2 object and query as params and
 * perform query, setting error flag in the event
 * of a db error.
 * @return null
*/
function run_query($db, $query) {
    syslog(LOG_INFO, $query);
    $affected = $db -> exec($query);
    if (MDB2::isError($affected)) {
        if (!defined('UNSUCCESSFUL_QUERY')) {
            define('UNSUCCESSFUL_QUERY', true);
        }
        echo "$query<br>\n";
        echo $affected -> getMessage() . "\n";
    }
}

/**
 * Check uploaded file for obvious problems
 * This function checks the uploaded file's
 * size, type, length, etc to make sure it's
 * worth continuing with processing
 * @return bool
*/
function check_uploaded_file($file_array) {
    if (!is_uploaded_file($file_array['tmp_name'])) {
        echo "File NOT uploaded OK<br>";
        die(upload_form());
    } elseif ($file_array['size'] < 1) {
        echo "File was empty";
        die(upload_form());
    } elseif ($file_array['error'] > 0) {
        echo "Uploading file encountered error #" . $file_array['error'];
        die(upload_form());
    } elseif ($file_array['type'] != 'text/xml') {
        echo "Expected file of type 'text/xml', but got " . $file_array['type'];
        die(upload_form());
    } else {
        //echo "File seems uploaded OK<br>";
        return true;
    }
}


if (!array_key_exists('confirmed', $_REQUEST)) {
    die(upload_form());
}

/*
foreach ($_REQUEST as $key => $val) {
echo "$key => $val <br>\n";
}
if (is_array($_FILES) && count($_FILES)>0) {
echo "<h2>FILES is an array</h2>";
print_r($_FILES);
}
*/


// no need to do anything till we check that the file's ok
if (check_uploaded_file($_FILES['file'])) {
    $xml_file = $_FILES['file']['tmp_name'];
    move_uploaded_file($tmp_file, $xml_file);
    //echo $xml_file . "\n<br>";
    $xml_str = sprintf('%s', file_get_contents($xml_file));
    //echo $xml_str;
}

$db = MDB2::connect(DEFAULT_DSN);
if (MDB2::isError($db)) {
    echo $db -> getMessage() . "\n";
}

$xml_obj = new SimpleXMLElement($xml_str);
foreach ($xml_obj -> context as $context) {
    $cn = $context['name'];
    $global_weight = 100;
    foreach ($context -> extension as $extension) {
        $en = $extension['name'];
        //printf("<pre>%s</pre>", print_r($extension, true));
        $ec = $extension['continue'];
        $global_weight+=100;
        foreach ($extension -> condition as $condition) {
            //print_r($condition);
            $cf = $condition['field'];
            $ce = addslashes($condition['expression']);
            //echo "<pre>Condidtion Expression for $en:\n    before: " . $condition['expression'] . "\n    after: $ce</pre>";
            $cb = $condition['break'];
            $weight = 0;
            foreach ($condition as $type => $action) {
                //echo "-------------------$type-----------------------------\n";
                $app_name = $action['application'];
                $app_data = addslashes($action['data']);
                $weight++;
                //echo "$cn\t$en\t$cf\t$ce\t$cc\t$app_name\t$app_data\t$ec\t$global_weight\t$weight\n";
                $query = sprintf('%s %s %s %s %s %s;',
                "INSERT INTO dialplan SET",
                "context='$cn', extension='$en', condition_field='$cf',",
                "condition_expression='$ce', application_name='$app_name',",
                "application_data='$app_data', weight='$weight', type='$type',",
                "ext_continue='$ec', global_weight='$global_weight',",
                "cond_break='$cb'"
                );
                run_query($db, $query);
            }
        }
    }
}
if (defined(UNSUCCESSFUL_QUERY) && UNSUCCESSFUL_QUERY == true) {
    echo "<h2>Some Queries Were Not Successful</h2>";
} else {
    echo "<h2>File Successfully Imported</h2>";
}
upload_form();

//printf("<pre>%s</pre>", print_r($xml_obj, true);
?>
