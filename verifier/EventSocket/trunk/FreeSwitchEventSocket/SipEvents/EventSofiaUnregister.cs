namespace FreeSwitch.EventSocket.SipEvents
{
    public class EventSofiaUnregister : EventBase
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

        public override string ToString()
        {
            return "SofiaUnregister(" + _user + "@" + _domain + ", " + _expires + ")." + base.ToString();
        }
    }
}
