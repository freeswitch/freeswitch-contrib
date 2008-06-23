namespace FreeSwitch.EventSocket
{
    #region enum ChannelState
    public enum ChannelState
    {
        New,           // Channel is newly created 
        Init,          // Channel has been initilized
        Routing,       // Channel is looking for a dialplan
        SoftExecute,   // Channel is in a passive transmit state
        Execute,       // Channel is executing it's dialplan 
        ExchangeMedia, // Channel is exchanging media
        Park,
        ConsumeMedia,  // Channel is consuming media
        Hibernate,     // Channel is in a sleep state
        Reset,
        Hangup,        // Channel is flagged for hangup and ready to end
        Done,          // Channel is ready to be destroyed and out of the state machine
        Unknown
    }
    #endregion

    public class ChannelInfo
    {
        private ChannelState _state;
        private int _stateNumber;
        private string _profile = string.Empty;
        private string _address = string.Empty;
        private string _hostName = string.Empty;
        private string _protocol = string.Empty;
        private string _readCodecName;
        private int _readCodecRate;
        private string _writeCodecName;
        private int _writeCodecRate;

        /// <summary>
        /// Coded used to read information from the channel.
        /// Example: L16
        /// </summary>
        public string ReadCodecName
        {
            get { return _readCodecName; }
            set { _readCodecName = value; }
        }

        /// <summary>
        /// Bitrate of the read coded that is used.
        /// Example: 8000
        /// </summary>
        public int ReadCodecRate
        {
            get { return _readCodecRate; }
            set { _readCodecRate = value; }
        }

        /// <summary>
        /// Codec used for writing to the channel.
        /// Example: L16
        /// </summary>
        public string WriteCodecName
        {
            get { return _writeCodecName; }
            set { _writeCodecName = value; }
        }

        /// <summary>
        /// Bitrate of the codec.
        /// Example: 8000
        /// </summary>
        public int WriteCodecRate
        {
            get { return _writeCodecRate; }
            set { _writeCodecRate = value; }
        }

        /// <summary>
        /// State that the channel is in. Check ChannelState enum for more information.
        /// </summary>
        public ChannelState State
        {
            get { return _state; }
            set { _state = value; }
        }

        /// <summary>
        /// Number of the state (from the enum in FreeSwitch)
        /// </summary>
        public int StateNumber
        {
            get { return _stateNumber; }
            set { _stateNumber = value; }
        }

        /// <summary>
        /// Name of the channel.
        /// </summary>
        /// <example>sofia/mydomain.com/1234@conference.freeswitch.org</example>
        /// <seealso cref="ChannelInfo.Protocol"/>
        /// <seealso cref="ProfileName"/>
        /// <seealso cref="Address"/>
        /// <seealso cref="HostName"/>
        public string Name
        {
            get { return _protocol + "/" + _profile + "/" + _address + "@" + _hostName; }
            set 
            { 
                string[] bits = value.Split('/');
                if (bits.Length == 3)
                {
                    _protocol = bits[0];
                    _profile = bits[1];
                    string[] userParts = bits[2].Split('@');
                    if (userParts.Length == 2)
                    {
                        _address = userParts[0];
                        HostName = userParts[1];
                    }
                    else
                        _address = bits[2];
                }
                
            }
        }

        /// <summary>
        /// ProfileName for the diaplan/sip.
        /// </summary>
        public string ProfileName
        {
            get { return _profile; }
            set { _profile = value; }
        }

        /// <summary>
        /// Extension/Username for the channel
        /// </summary>
        public string Address
        {
            get { return _address; }
            set { _address = value; }
        }

        /// <summary>
        /// Where the extension/user is calling from.
        /// </summary>
        public string HostName
        {
            get { return _hostName; }
            set
            {
                if (value == null)
                {
                    _hostName = string.Empty;
                    return;
                }

                int pos = value.IndexOf(':');
                if (pos == -1)
                    _hostName = value;
                else
                    _hostName = value.Substring(0, pos);
            }
        }

        /// <summary>
        /// Protocol used for communication.
        /// "sofia" = sip.
        /// </summary>
        public string Protocol
        {
            get { return _protocol;  }
        }

        public bool Parse(string name, string value)
        {
            switch (name)
            {
                case "channel-state":
                    _state = StateFromString(value);
                    break;
                case "channel-state-number":
                    int.TryParse(value, out _stateNumber);
                    break;
                case "channel-name":
                    Name = value;
                    break;
                case "channel-read-codec-name":
                    _readCodecName = value;
                    break;
                case "channel-read-codec-rate":
                    int.TryParse(value, out _readCodecRate);
                    break;
                case "channel-write-codec-name":
                    _writeCodecName = value;
                    break;
                case "channel-write-codec-rate":
                    int.TryParse(value, out _writeCodecRate);
                    break;
                default:
                    return false;
            }
            return true;
        }

        public ChannelState StateFromString(string state)
        {
            switch (state)
            {
                case "CS_NEW": return ChannelState.New;
                case "CS_INIT": return ChannelState.Init;
                case "CS_ROUTING": return ChannelState.Routing;
                case "CS_SOFT_EXECUTE": return ChannelState.SoftExecute;
                case "CS_EXECUTE": return ChannelState.Execute;
                case "CS_EXCHANGE_MEDIA": return ChannelState.ExchangeMedia;
                case "CS_PARK": return ChannelState.Park;
                case "CS_CONSUME_MEDIA": return ChannelState.ConsumeMedia;
                case "CS_HIBERNATE": return ChannelState.Hibernate;
                case "CS_HANGUP": return ChannelState.Hangup;
                case "CS_DONE": return ChannelState.Done;
                case "CS_RESET": return ChannelState.Reset;
                default: return ChannelState.Unknown;
            }
        }

        public string StateToString(ChannelState state)
        {
            return "Unknown";
        }

        public override string ToString()
        {
            return _address + "@" + _hostName + ": " + _state;
        }
    }
}
