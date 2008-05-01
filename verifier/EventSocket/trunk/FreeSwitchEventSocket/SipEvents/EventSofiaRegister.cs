namespace FreeSwitch.EventSocket
{
    /*
     * profile-name: gauffin.com
from-user: jonas
from-host: gauffin.com
contact: %22Jonas%20Gauffin%22%20%3Csip%3Ajonas%40192.168.1.101%3A5060%3E
*/

    /// <summary>
    /// Sofia_reg.c way down in sofia_reg_handle_register
    /// </summary>
    public class EventSofiaRegister : EventBase
    {
        private string _domain;
        private string _profileName;
        private string _user;
        private string _contact;
        private string _callId;
        private int _expires;

        public string ProfileName
        {
            get { return _profileName; }
            set { _profileName = value; }
        }

        public string UserName
        {
            get { return _user; }
            set { _user = value; }
        }

        public string Domain
        {
            get { return _domain; }
            set { _domain = value; }
        }

        public string Contact
        {
            get { return _contact; }
            set { _contact = value; }
        }

        public string CallId
        {
            get { return _callId; }
            set { _callId = value; }
        }

        public int Expires
        {
            get { return _expires; }
            set { _expires = value; }
        }

        /*
			switch_event_add_header(s_event, SWITCH_STACK_BOTTOM, "profile-name", "%s", profile->name);
			switch_event_add_header(s_event, SWITCH_STACK_BOTTOM, "from-user", "%s", to_user);
			switch_event_add_header(s_event, SWITCH_STACK_BOTTOM, "from-host", "%s", to_host);
			switch_event_add_header(s_event, SWITCH_STACK_BOTTOM, "rpid", "%s", rpid);
        */        
        public override bool ParseCommand(string name, string value)
        {
            switch (name)
            {
                case "profile-name":
                    _profileName = value;
                    break;
                case "from-user":
                    _user = value;
                    break;
                case "from-host":
                    _domain = value;
                    break;
                case "contact":
                    _contact = value;
                    break;
                case "call-id":
                    _callId = value;
                    break;
                case "expires":
                    int.TryParse(value, out _expires);
                    break;
                default:
                    return base.ParseCommand(name, value);
            }
            return true;
        }
    }
}