namespace FreeSwitch.EventSocket
{
    public class EventSofiaExpire : EventPresence
    {
        private string _profileName;
        private string _userName;
        private string _domain;
        private string _userAgent;

        public string ProfileName
        {
            get { return _profileName; }
            set { _profileName = value; }
        }

        public string UserName
        {
            get { return _userName; }
            set { _userName = value; }
        }

        public string Domain
        {
            get { return _domain; }
            set { _domain = value; }
        }

        public string UserAgent
        {
            get { return _userAgent; }
            set { _userAgent = value; }
        }

        /*
				switch_event_add_header(s_event, SWITCH_STACK_BOTTOM, "profile-name", "%s", argv[6]);
			switch_event_add_header(s_event, SWITCH_STACK_BOTTOM, "user", "%s", argv[1]);
			switch_event_add_header(s_event, SWITCH_STACK_BOTTOM, "host", "%s", argv[2]);
			switch_event_add_header(s_event, SWITCH_STACK_BOTTOM, "user-agent", "%s", argv[5]);	
         * */
        public override bool ParseCommand(string name, string value)
        {
            switch (name)
            {
                case "profile-name":
                    _profileName = value;
                    break;
                case "user":
                    _userName = value;
                    break;
                case "host":
                    _domain = value;
                    break;
                case "user-agent":
                    _userAgent = value;
                    break;
                default:
                    return base.ParseCommand(name, value);
            }
            return true;

        }

        public override string ToString()
        {
            return "SofiaExpire(" + _userName + "@" + _domain + ")." + base.ToString();
        }
    }
}
