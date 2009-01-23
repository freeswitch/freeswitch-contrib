import logging

from freeswitch.lib.base import *




log = logging.getLogger(__name__)

class FreeswitchdirectoryController(BaseController):
    
    bootReply = '''<?xml version="1.0" encoding="UTF-8" standalone="no"?>\n'''\
    '''<document type="freeswitch/xml">\n'''\
    '''<section name="directory" description="arbitrary stuff here">\n'''\
    '''</section>\n'''\
    '''</document>'''
    
    error = '''<?xml version="1.0" encoding="UTF-8" standalone="no"?>\n'''\
    '''<document type="freeswitch/xml">\n'''\
    '''<section name="dialplan" description="RE Dial Plan For FreeSwitch">\n'''\
    '''</section>\n'''\
    '''</document>'''
    
    def user(self, didNumber, domain, mailbox_id):
        test = '''<?xml version="1.0" encoding="UTF-8" standalone="no"?>\n'''\
        '''<document type="freeswitch/xml">\n'''\
        '''<section name="directory" description="arbitrary stuff here">\n'''\
        '''<domain name="%s">\n'''\
        '''<groups>\n'''\
        '''<group name="default">\n'''\
        '''<users>\n'''\
        '''<user id="%s" mailbox="%s">\n'''\
        '''<params>\n'''\
        '''<param name="password" value="1234"/>\n'''\
        '''<param name="vm-password" value="0000"/>\n'''\
        '''</params>\n'''\
        '''<variables>\n'''\
        '''<variable name="accountcode" value="%s"/>\n'''\
        '''<variable name="user_context" value="default"/>\n'''\
        '''<variable name="vm_extension" value="%s"/>\n'''\
        '''<variable name="max_calls" value="1"/>\n'''\
        '''<variable name="fail_over" value="415"/>\n'''\
        '''<variable name="cringback" value="us-ring"/>\n'''\
        '''</variables>\n'''\
        '''</user>\n'''\
        '''</users>\n'''\
        '''</group>\n'''\
        '''</groups>\n'''\
        '''</domain>\n'''\
        '''</section>\n'''\
        '''</document>''' % (domain, mailbox_id, mailbox_id, mailbox_id, mailbox_id)
        
        return test
    
    
    # a very simple reply that creates a user for the voicemail.
    def directory(self):
        
        try:
            return self.user(request.POST.getone('mailbox'), request.POST.getone('domain'), request.POST.getone('user'))
        
        except KeyError, e:
            pass
        
        return self.bootReply


