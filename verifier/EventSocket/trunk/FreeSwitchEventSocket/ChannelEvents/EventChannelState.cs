namespace FreeSwitch.EventSocket
{
    public class EventChannelState : ChannelEvent
    {
        private PartyInfo _caller = new PartyInfo();
        private PartyInfo _originator = new PartyInfo();
        private bool _screenBit = false;

        /// <summary>
        /// Information about the called party.
        /// This property is null for certain channel states,
        /// depending on if freeswitch sends it or not.
        /// </summary>
        public PartyInfo Caller
        {
            get { return _caller; }
            set { _caller = value; }
        }

        /// <summary>
        /// Information about the destination (place that the caller want to reach).
        /// This property is null for certain channel states,
        /// depending on if freeswitch sends it or not.
        /// </summary>
        public PartyInfo Originator
        {
            get { return _originator; }
            set { _originator = value; }
        }

        public bool ScreenBit
        {
            get { return _screenBit; }
            set { _screenBit = value; }
        }

        public override bool ParseCommand(string name, string value)
        {
            if (name == "screen-bit")
            {
                ScreenBit = value == "yes";
                return true;
            }
            else if (name.Length > 11 && name.Substring(0, 11) == "originator-"
                     || name.Length > 11 && name.Substring(0, 11) == "originatee-")
            {
                if (_originator == null)
                    _originator = new PartyInfo();

                return _originator.Parse(name.Substring(11), value);
            }
            else if (name.Length > 7 && name.Substring(0, 7) == "caller-")
            {
                if (_caller == null)
                    _caller = new PartyInfo();

                return _caller.Parse(name.Substring(7), value);
            }
            else
                return base.ParseCommand(name, value);
        }
    }
}