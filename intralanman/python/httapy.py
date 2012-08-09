"""
An object oriented abtraction interface for creating XML documents to be used with FreeSWITCH's mod_httapi

Create dynamic call routing and IVR scenarios from this easy to use abstraction layer to the mod_httapi document model for FreeSWITCH.
Author: Raymond Chandler (intralanman@freeswitch.org)
"""
import xml.etree.ElementTree
import re

class httapy:
    """
    This is the base class of the httapi object
    """

    doc = xml.etree.ElementTree.Element('document', type='text/freeswitch-httapi')
    cur = None
    work = "not started"
    paramsElement = xml.etree.ElementTree.SubElement(doc, 'params')
    varsElement = xml.etree.ElementTree.SubElement(doc, 'variables')
    workElement = xml.etree.ElementTree.SubElement(doc, 'work')

    def output(self):
        """
        This method will output the built-up XML document
        """
        if self.work == "ended":
            print xml.etree.ElementTree.tostring(self.doc)

    def open(self, tag, parent=None):
        """
        This method starts a new XML element.  If parent is ommitted, the new element will be a child of the work element.
        """
        if parent is None:
            parent = self.workElement
        self.cur = xml.etree.ElementTree.SubElement(parent, tag)

    def close(self):
        """
        This method is here strictly for API compatibility with the PHP version of this class
        """
        return

    def start_work(self):
        """
        This method is currently only here for API compatibility with
        the PHP version of this class, but will do some error checking
        in the future to make sure that you don't try to start_work
        multiple times, etc
        """
        if self.work == "not started":
            self.work = "started"

    def end_work(self):
        """
        This method is currently only here for API compatibility with
        the PHP version of this class, but will do some error checking
        in the future to make sure that you don't try to end_work
        multiple times, etc
        """
        if self.work == "started":
            self.work = "ended"

    def start_variables(self):
        """
        This method is here strictly for API compatibility with the PHP version of this class
        """
        return

    def add_variable(self, var, val):
        """
        This method will add a variable to the XML document and, by association, the call on which it was requested
        """
        self.open(var, self.varsElement)
        self.cur.text(val)

    def end_variables(self):
        """
        This method is here strictly for API compatibility with the PHP version of this class
        """
        self.close()

    def start_params(self):
        """
        This method is here strictly for API compatibility with the PHP version of this class
        """
        return

    def add_param(self, name, value):
        """
        This method will add a param to the XML document
        """
        self.open(name, self.paramsElement)
        self.cur.text(value)

    def end_params(self):
        """
        This method is here strictly for API compatibility with the PHP version of this class
        """
        self.close()

    def add_action(self, action):
        """
        This method will add an object of type httapy_action or any of it's subclasses to the XML document
        """
        if isinstance(action, httapy_action) or issubclass(action, httapy_action):
            self.workElement.append(action.XMLElement)

class httapy_action():
    """
    This is where the different action types get created.
    This class shouldn't be used directly, but be only a base
    for the many different httapi action types
    """
    allow_bindings = False
    require_text = False
    action_text = ''
    action_name = ''
    attributes = []
    defined_attributes = {}
    required_attributes = []
    XMLElement = None

    def __init__(self, text=None):
        """
        Create an object based on the type of action
        """
        if self.require_text and text is None:
            raise Exception("%s requires an argument" % self.__class__.__name__)

        if self.action_name:
            self.XMLElement = xml.etree.ElementTree.Element(self.action_name)
        else:
            aname = re.sub('^httapy_', '', self.__class__.__name__)
            aname = re.sub('_', '-', aname)
            self.XMLElement = xml.etree.ElementTree.Element(aname)

    def __getattr__(self, item):
        """
        Most of the methods won't exist, since their only purpose is to
        add attributes to the XML element and we'd like to do that via
        simple method calls, then we do it this way.
        """
        def function(value=None):
            if not value:
                raise Exception("%s requires an argument" % item)

            dashed_key = re.sub('_', '-', item)
            if dashed_key not in self.attributes:
                raise NotImplementedError("%s isn't implemented in %s" % (dashed_key, self.__class__.__name__))

            self.attr(dashed_key, value)
        return function

    def attr(self, key, val):
        """
        Set an attribute named "key" to "value"
        """
        self.defined_attributes[key] = val
        self.XMLElement.attrib[key] = val

    def name(self, name):
        """
        Set the name atribute
        """
        self.attr('name', name)

    def action(self, data):
        """
        Set the action attribute
        """
        self.attr('action', data)

    def temp_action(self, data):
        """
        Set the temp-action attribute
        """
        print "temp-action\n"

    def add_binding(self, binding):
        """
        Add an object of type httapy_action_binding to the action object
        """
        if not self.allow_bindings:
            raise NotImplementedError("%s doesn't allow bindings" % self.__class__.__name__)
        self.XMLElement.append(binding.XMLElement)


class httapy_prompt(httapy_action):
    allow_bindings = True
    require_text = False


class httapy_break(httapy_action):
    require_text = False


class httapy_conference(httapy_action):
    """
    Enter a conference
    """
    require_text = False
    attributes = ['profile']
    required_attributes = ['profile']


class httapy_continue(httapy_action):
    require_text = False


class httapy_dial(httapy_action):
    """
    Hit the dialplan with the digits entered
    """
    require_text = True
    attributes = ['context', 'dialplan', 'caller_id_name', 'caller_id_number']


class httapy_execute(httapy_action):
    """
    Execute an application for the current call (requires permissions from httapi.conf.xml)
    """
    require_text = False
    attributes = ['application']
    required_attributes = ['application']


class httapy_getVariable(httapy_action):
    """
    Get a channel variable passed back in the next request (requires permissions from httapi.conf.xml)
    """
    require_text = True
    attributes = ['permanent']


class httapy_hangup(httapy_action):
    """
    Hangup the call
    """
    require_text = False
    attributes = ['cause']


class httapy_log(httapy_action):
    require_text = False
    attributes = ['level', 'clean']

class httapy_pause(httapy_action):
    """
    Sleep
    """
    require_text = False
    attributes = ['milliseconds']
    required_attributes = ['milliseconds']


class httapy_playback(httapy_prompt):
    """
    Play a file
    """
    require_text = False
    action_name = 'playback'
    attributes = ['file', 'error-file', 'digit-timeout', 'input-timeout', 'loops', 'asr-engine', 'asr-grammar']
    required_attributes = ['file', 'name']


class httapy_record(httapy_action):
    require_text = False
    attributes = ['file', 'error-file', 'digit-timeout', 'input-timeout', 'loops']
    required_attributes = ['file']


class httapy_recordCall(httapy_action):
    require_text = False
    attributes = ['limit']


class httapy_say(httapy_action):
    require_text = False
    attributes = ['gender', 'method', 'type', 'language']


class httapy_speak(httapy_action):
    require_text = False
    attributes = ['engine', 'voice', 'digit-timeout', 'input-timeout', 'loops']


class httapy_sms(httapy_action):
    require_text = True
    attributes = ['to']


class httapy_voicemail(httapy_action):
    """
    call the voicemail app without requiring execute permissions (useful for doing PIN-based authentication)
    """
    require_text = False
    attributes = ['auth-only', 'id', 'domain', 'profile', 'check']


class httapy_google_tts(httapy_playback):
    """
    An extension to the httapy_playback class that uses a url to Google's TTS service as the file to play (requires mod_shout)
    """
    require_text = False
    required_attributes = ['file', 'name']

    def text(self, to_speak):
        """
        The text to speak.  This will be automatically turned into a proper url to the Google TTS service
        """
        to_speak = re.sub('\s', '+', to_speak)
        url = "http://(ext=mp3,user_agent='Mozilla 5.0 (X11; Linux x86_64; rv:9.0.1) Gecko/20100101 Firefox/9.0.1')translate.google.com/translate_tts?q=%s" % (to_speak)
        self.file(url)


""" skeleton for new actions
class httapy_(httapy_action):
    required_attributes = []
"""


class httapy_action_binding():
    """
    This class will create the binding(s) for an action
    """
    XMLElement = None

    def __init__(self, digits):
        """
        Create bind XML element with data of "digits"
        """
        self.XMLElement = xml.etree.ElementTree.Element('bind')
        self.XMLElement.text = digits

    def strip(self, strippers):
        """
        The characters to strip from the entered string (usually just #)
        """
        self.XMLElement.attrib['strip'] = strippers


if __name__ == '__main__':

    obj = httapy()

    obj.start_work()

    # create the pause obj
    pause = httapy_pause()
    pause.milliseconds("5000")

    # create the tts obj
    pbind1 = httapy_action_binding('1')
    pbind1.strip('#')
    pbind2 = httapy_action_binding('2')

    play = httapy_google_tts()
    play.text("to do something, press 1, to do something else, press 2")
    play.name('option')
    play.input_timeout('5000')
    play.add_binding(pbind1)
    play.add_binding(pbind2)


    # add our actions to the httapy object
    obj.add_action(pause)
    obj.add_action(play)

    obj.end_work()

    obj.output()

