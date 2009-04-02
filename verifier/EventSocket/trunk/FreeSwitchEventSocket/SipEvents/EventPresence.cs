namespace FreeSwitch.EventSocket
{
    public class EventPresence : SipEvent
    {
        private PartyInfo _caller;
        private string _rpid = string.Empty;
        private string _status = string.Empty;
        //private string _callId; // not "presencein/register", "presenceout/register"
        //private string _contact; // not "presencein/register", "presenceout/register"
        //private string _expires; //not "presencein/register", "presenceout/register"
        private string _eventType = string.Empty;
        private string _login = string.Empty;
        private EventChannelState _channelState = new EventChannelState();

        public string Login
        {
            get { return _login; }
            set { _login = value; }
        }

        /// <summary>
        /// "Click to call", "Registered", "unavailable", "Active (%d waiting)", "Idle"
        /// </summary>
        public string Status
        {
            get { return _status; }
            set { _status = value; }
        }

        public string Rpid
        {
            get { return _rpid; }
            set { _rpid = value; }
        }

        /// <summary>
        /// Caller is only specified on state CS_ROUTING
        /// and not on CS_HANGUP
        /// </summary>
        public PartyInfo Caller
        {
            get { return _caller; }
            set { _caller = value; }
        }

        /// <summary>
        /// presence, 
        /// </summary>
        public string PresenceEventType
        {
            get { return _eventType; }
            set { _eventType = value; }
        }

        /// <summary>
        /// We *may* have channel state info
        /// </summary>
        public EventChannelState ChannelState
        {
            get { return _channelState; }
            set { _channelState = value; }
        }


        /*
        proto: sip
        login: sip%3Amod_sofia%40192.168.0.58%3A5070
        rpid: unknown
        from: gauffin%40gauffin.com
        status: Registered
         * */
        public override bool ParseCommand(string name, string value)
        {
            bool res = _channelState.ParseCommand(name, value);
            switch (name)
            {
                case "status":
                    _status = value;
                    break;
                case "rpid":
                    _rpid = value;
                    break;
                case "login":
                    _login = value;
                    break;
                case "event_type":
                    _eventType = value;
                    break;

                default:

                    if (name.Length > 7 && name.Substring(0, 7) == "caller-")
                    {
                        if (Caller == null)
                            Caller = new PartyInfo();
                        return Caller.Parse(name.Substring(7), value);
                    }

                    return base.ParseCommand(name, value) || res;
            }
            return true;
        }

        public override string ToString()
        {
            return "Presence(" + _login + ", " + _status + ")." + _channelState + "." + base.ToString();
        }
    }
}