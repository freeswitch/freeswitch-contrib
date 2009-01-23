import logging

from freeswitch.lib.base import *



log = logging.getLogger(__name__)

class FreeswitchdialplanController(BaseController):
    

    

    # needs to be checked what's the best return string when an error occures
    error = '''<?xml version="1.0" encoding="UTF-8" standalone="no"?>\n'''\
    '''<document type="freeswitch/xml">\n'''\
    '''<section name="dialplan" description="RE Dial Plan For FreeSwitch">\n'''\
    '''</section>\n'''\
    '''</document>'''


    
    # the name forward is a bit confusing as it is actually a FS bridge
    # the next two methods allow you to forward a single call to n number of outbound calls
    def forwardCreate(self, number, gateway, forwardString):
        if forwardString != '':
            forwardString = forwardString + ','
            
        return forwardString + '''sofia/gateway/%s/%s''' % (gateway, number)
        
        
        
    def forwardFinishCreate(self, didNumber, forwardString):
        return '''<?xml version="1.0" encoding="UTF-8" standalone="no"?>\n'''\
        '''<document type="freeswitch/xml">\n'''\
        '''<section name="dialplan" description="RE Dial Plan For FreeSwitch">\n'''\
        '''<context name="public">\n'''\
        '''<extension name="extension%s">\n'''\
        '''<condition field="destination_number" expression="^(%s)$">\n'''\
        '''<action application="bridge" data="%s"/>\n'''\
        '''</condition>\n'''\
        '''</extension>\n'''\
        '''</context>\n'''\
        '''</section>\n'''\
        '''</document>\n''' % (didNumber, didNumber, forwardString)
    
    
    # bridge one inbound call with a single outbound call
    def bridge(self, didNumber, outboundNumber, gateway):
        
        return '''<?xml version="1.0" encoding="UTF-8" standalone="no"?>\n'''\
        '''<document type="freeswitch/xml">\n'''\
        '''<section name="dialplan" description="RE Dial Plan For FreeSwitch">\n'''\
        '''<context name="public">\n'''\
        '''<extension name="extension%s">\n'''\
        '''<condition field="destination_number" expression="^(%s)$">\n'''\
        '''<action application="bridge" data="sofia/gateway/%s/%s"/>\n'''\
        '''</condition>\n'''\
        '''</extension>\n'''\
        '''</context>\n'''\
        '''</section>\n'''\
        '''</document>\n''' % (didNumber, didNumber, gateway, outboundNumber)


      
    def voicemail(self, didNumber, mailbox_id):
        
        return '''<?xml version="1.0" encoding="UTF-8" standalone="no"?>\n'''\
        '''<document type="freeswitch/xml">\n'''\
        '''<section name="dialplan" description="RE Dial Plan For FreeSwitch">\n'''\
        '''<context name="public">\n'''\
        '''<extension name="voicemail%s">\n'''\
        '''<condition field="destination_number" expression="^(%s)$">\n'''\
        '''<action application="voicemail" data="default $${domain} %s"/>\n'''\
        '''</condition>\n'''\
        '''</extension>\n'''\
        '''</context>\n'''\
        '''</section>\n'''\
        '''</document>''' % (didNumber, didNumber, mailbox_id)
        
    def checkVoicemail(self, didNumber, mailbox_id):
        
        return '''<?xml version="1.0" encoding="UTF-8" standalone="no"?>\n'''\
        '''<document type="freeswitch/xml">\n'''\
        '''<section name="dialplan" description="RE Dial Plan For FreeSwitch">\n'''\
        '''<context name="public">\n'''\
        '''<extension name="voicemail%s">\n'''\
        '''<condition field="destination_number" expression="^(%s)$">\n'''\
        '''<action application="voicemail" data="check default $${domain} %s"/>\n'''\
        '''</condition>\n'''\
        '''</extension>\n'''\
        '''</context>\n'''\
        '''</section>\n'''\
        '''</document>''' % (didNumber, didNumber, mailbox_id)


    # if you hungup the call before it has been answered a busy is issued
    def busy(self, didNumber):
        return '''<?xml version="1.0" encoding="UTF-8" standalone="no"?>\n'''\
        '''<document type="freeswitch/xml">\n'''\
        '''<section name="dialplan" description="RE Dial Plan For FreeSwitch">\n'''\
        '''<context name="public">\n'''\
        '''<extension name="extension%s">\n'''\
        '''<condition field="destination_number" expression="^(%s)$">\n'''\
        '''<action application="hungup" data=""/>\n'''\
        '''</condition>\n'''\
        '''</extension>\n'''\
        '''</context>\n'''\
        '''</section>\n'''\
        '''</document>\n''' % (destinationNumber, didNumber)




    def dialplan(self):
        
        action = "voicemail"
        
        # replace with your sofia gateway
        gateway = 'sipgate2'
        
        # replace with phone numbers
        forwards = ['the did number extension - gateway', 'the number to forward to']
        
        # that is the username of the mailbox
        mailbox_id = 'test'
        
        
        didNumber = str(request.POST.getone('Caller-Destination-Number'))
        

        # ------------------------------------------------------------ Forward -------------------------------------------------------
        # if the action is forward, we forward to all numbers in forwards
        if action == "forward":
            
            forwardString = ''
            for f in forwards:
                forwardString = self.forwardCreate(f, gateway, forwardString)

            return self.forwardFinishCreate(didNumber, forwardString)



        elif action == "checkVoicemail":   
                
            return self.checkVoicemail(didNumber, mailbox_id)
            

        
        elif action == "voicemail":   
                
            return self.voicemail(didNumber, mailbox_id)
        
        
