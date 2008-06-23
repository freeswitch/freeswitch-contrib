namespace FreeSwitch.EventSocket
{
    public class EventChannelState : ChannelEvent
    {
        private const string Yes = "yes";
        private const string CallerTag = "caller-";
        private const string OriginatorTag = "originator-";
        private const string OriginateeTag = "originatee-";
        private const string OtherLegTag = "other-leg-";
        private const string ScreenBitTag = "screen-bit";
        private PartyInfo _caller = PartyInfo.Empty;
        private PartyInfo _originator = PartyInfo.Empty;
        private PartyInfo _otherLeg = PartyInfo.Empty;
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

        /// <summary>
        /// Other leg of call.
        /// </summary>
        public PartyInfo OtherLeg
        {
            get { return _otherLeg; }
            set { _otherLeg = value; }
        }

        public override bool ParseCommand(string name, string value)
        {
            if (name == ScreenBitTag)
            {
                ScreenBit = value == Yes;
                return true;
            }
            else if (name.Length > 11 && name.Substring(0, 11) == OriginatorTag
                     || name.Length > 11 && name.Substring(0, 11) == OriginateeTag)
            {
                if (_originator == PartyInfo.Empty)
                    _originator = new PartyInfo();

                return _originator.Parse(name.Substring(11), value);
            }
            else if (name.Length > 7 && name.Substring(0, 7) == CallerTag)
            {
                if (_caller == PartyInfo.Empty)
                    _caller = new PartyInfo();

                return _caller.Parse(name.Substring(7), value);
            }
            else if (name.Length > 10 && name.Substring(0, 10) == OtherLegTag)
            {
                if (_otherLeg == PartyInfo.Empty)
                    _otherLeg = new PartyInfo();

                return _otherLeg.Parse(name.Substring(10), value);
            }
            else
                return base.ParseCommand(name, value);
        }

        public override string ToString()
        {
            string temp = "ChannelState(";
            if (Caller != PartyInfo.Empty)
                temp += " Caller{" + Caller + "}";
            if (Originator != PartyInfo.Empty)
                temp += " Originator{" + Originator + "}";
            if (OtherLeg != PartyInfo.Empty)
                temp += " OtherLeg{" + OtherLeg + "}";
            return temp + ")." + base.ToString();
        }
    }
}