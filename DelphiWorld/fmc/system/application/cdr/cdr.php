<?php
  include_once('Parse_XML_CDR.php');
  Header("Content-type: text/plain");
  $xml = new Parse_CDR_XML($_POST['cdr']);
  $cdr=$xml->ReturnArray();
  // log the entries
  // it would probably be better to log to a database but simple examples dont do that
  $fh = fopen("xml/cdr.xml.".uniqid(), 'wb');
  fwrite($fh, $_POST['cdr'] . "\n\nprint_r\n\n");
  fwrite($fh,print_r($cdr,true)."\n\n");
  fclose($fh);
?>