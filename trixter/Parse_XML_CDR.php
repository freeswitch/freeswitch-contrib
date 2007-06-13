<?php //  -*- mode:c; tab-width:4; c-basic-offset:4; c-indent-level:4; indent-tabs-mode:nil;  -*-

class Parse_CDR_XML {
    var $_data = Array();
    var $_name = Array();
    var $_rep = Array();
    var $_parser = 0;
    var $_showAttribs;
    var $_level = 0;
    
    function Parse_CDR_XML(&$data,$showattribs = false) {
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
        return $this->_data[0];
    }
    
    
    function & ReturnAttributes() {
        return $this->_data['_Attributes_'];
    }
    
    
    function _startElement($parser, $name, $attrs){
        if (!isset($this->_rep[$name])) $this->_rep[$name] = 0;
        if ($name != "cdr") {
            $this->_addElement($name,$this->_data[$this->_level],$attrs,true);
            $this->_name[$this->_level] = $name;
            $this->_level++;
        }
        
    }
    
    
    function _endElement($parser,$name){
        if (isset($this->_name[$this->_level - 1])) {
            if (isset($this->_data[$this->_level])){
                $this->_addElement($this->_name[$this->_level - 1],$this->_data[$this->_level - 1],$this->_data[$this->_level],false);
            }
            unset($this->_data[$this->_level]);
            $this->_level--;
            $this->_rep[$name]++;
        }
    }
    
    
    
    function _cdata($parser, $data) {
        if ($this->_name[$this->_level - 1]) {
            $this->_addElement($this->_name[$this->_level - 1],$this->_data[$this->_level - 1],$data,false);
        }
    }
    
    
    function _addElement(&$name,&$start,$add = array(),$isattribs = false) {
        if (((sizeof($add) == 0 && is_array($add)) || !$add) && !is_numeric($add)) {
            if (!isset($start[$name])) $start[$name] = '';
            $add = '';
        }
        $update = &$start[$name];

        if ($isattribs && !$this->_showAttribs) return;
        elseif ($isattribs) $this->_data['_Attributes_'][$this->_level][$name][] = $add;
        elseif (is_array($add) && is_array($update)) $update += $add;
        elseif (is_array($update)) return;
        elseif (is_array($add)) $update = $add;
        elseif ($add || is_numeric($add)) $update .= "$add";
    }

}
?>
