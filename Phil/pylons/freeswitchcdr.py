import logging

from freeswitch.lib.base import *


import xml
from xml.sax.handler import ContentHandler 


log = logging.getLogger(__name__)


class FreeswitchcdrController(BaseController):
    

    # simple method to display the uuid of a call that got terminated
    def cdr(self):
        
        next = 0
        uuid = []
              
        try:
            cdr = request.POST.getone('cdr')
            # needs to be casted to a string as the unicode string can't be parsed correctly by the sax parser
            cdr = str(cdr)
            
            cdrHandler = CDR_XML_Handler()
            xml.sax.parseString(cdr, cdrHandler) 
            uuid = cdrHandler.uuid
            print uuid

            
        except KeyError, e:
            print e
            return

        
        
        
class CDR_XML_Handler(ContentHandler):    

    def __init__(self):
        self.uuid = []
        self.uuidString = ""
        self.isUuidElement = 0
        
    def startElement(self, name, attrs):

        if name == 'uuid':
            self.isUuidElement = 1
            uuidString = ""
    
    def characters(self, ch):
        if self.isUuidElement == 1:
            self.uuidString += ch

    def endElement(self, name):
        if name == 'uuid':
            self.isUuidElement= 0 
            self.uuid.append(self.uuidString)
