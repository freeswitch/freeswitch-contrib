<?php //  -*- mode:c; tab-width:4; c-basic-offset:4; c-indent-level:4; indent-tabs-mode:nil;  -*-

class Parse_CDR_XML {
    var $_data = Array();
    var $_name = Array();
    var $_rep = Array();
    var $_parser = 0;
    var $_showAttribs;
    var $_level = 0;
    var $currentTag;
    var $_CDR=array();
    var $_callflowcnt=0;
    var $_iscallflow=0;
    
    function Parse_CDR_XML(&$data,$showattribs = false) {
        $data = stripslashes(preg_replace('/<.xml (.*?)>(.*)/',"\\2",$data));
        $data = preg_replace("/<(.*?)><(.*?)><\/(.*?)>/","<\\1>\\2</\\3>",$data);
        $this->_showAttribs = $showattribs;
        $this->_parser = xml_parser_create();
        
        xml_set_object($this->_parser,$this);
        
        xml_parser_set_option($this->_parser,XML_OPTION_CASE_FOLDING,false);
        
        xml_set_element_handler($this->_parser, "_startElement", "_endElement");
        xml_set_character_data_handler($this->_parser, "_cdata");
        
        $this->_data = array();
        $this->_level = 0;
        if(!xml_parse($this->_parser, $data, true)){
            return false;
        }
        xml_parser_free($this->_parser);
    }
    
    
    function & ReturnArray() {
        return $this->_CDR;
    }
    
    
    function & ReturnAttributes() {
        return $this->_data['_Attributes_'];
    }
    
    
    function _startElement($parser, $name, $attrs){
        $this->_ElementName="";
        if (!isset($this->_rep[$name])) $this->_rep[$name] = 0;
        switch ($name) {
        case "cdr":
            break;
        case "app_log":
        case "variables":
            if(!isset($this->_CDR[$name])) {
                $this->_CDR[$name]=array();
            }
            $this->currentTag = $name;
            break;
        case "application":
            if($this->_iscallflow) {
                $this->_CDR[$this->currentTag][$this->_callflowcnt]['extension']['application'][]=$attrs;
            } else {
                $this->_CDR[$this->currentTag][]=$attrs;
            }
            break;
        case "callflow":
            $this->_CDR[$name]=array();
            $this->currentTag = $name;
            foreach ($attrs as $key => $value) {
                $this->_CDR[$this->currentTag][$this->_callflowcnt][$key]=$value;
            }
            $this->_iscallflow=1;
            break;
        case "extension":
            foreach ($attrs as $key => $value) {
                $this->_CDR[$this->currentTag][$this->_callflowcnt]['extension'][$key]=$value;
            }
            break;
        case "originator":
            if($this->_iscallflow) {
                $this->currentTag = $name;
            } else {
                $this->_ElementName=$name;
            }
            break;
        case "caller_profile":
        case "times":
        case "originator_caller_profile":
            $this->currentTag = $name;
            break;
        default:
            $this->_ElementName=$name;
            break;
        }
        
    }
    
    
    function _endElement($parser,$name){
        $this->_ElementName="";
        switch ($name) {
        case "callflow":
            $this->_callflowcnt++;
            $this->_iscallflow=0;
            break;
        }
        
    }
    
    
    
    function _cdata($parser, $data) {
        switch($this->currentTag) {
        case "cdr":
            return;
            break;
        case "app_log":
            return;
            break;
        case "originator_caller_profile":
            if(isset($data) && $this->_ElementName != "" &&
               !isset($this->_CDR[$this->currentTag][$this->_ElementName])
               ) {

                $this->_CDR['callflow'][$this->_callflowcnt]['originator'][$this->currentTag][$this->_ElementName]=$data;
            }
            return;
            break;
        case "caller_profile":
        case "times":
            if(isset($data) && $this->_ElementName != "" &&
               !isset($this->_CDR[$this->currentTag][$this->_ElementName])
               ) {

                $this->_CDR['callflow'][$this->_callflowcnt][$this->currentTag][$this->_ElementName]=$data;
            }
            return;
            break;
        }
        if(isset($data) && $this->_ElementName != "" &&
           !isset($this->_CDR[$this->currentTag][$this->_ElementName])
           ) {
            $this->_CDR[$this->currentTag][$this->_ElementName]=$data;
        }
    }
    
}
?>
