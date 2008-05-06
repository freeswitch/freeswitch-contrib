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
        ConsumeMedia,  // Channel is consuming media
        Hibernate,     // Channel is in a sleep state
        Hangup,        // Channel is flagged for hangup and ready to end
        Done,          // Channel is ready to be destroyed and out of the state machine
        Unknown
    }
    #endregion

    public class ChannelInfo
    {
        private ChannelState m_state;
        private int m_stateNumber;
        private string _profile;
        private string m_address;
        private string _hostName;
        private string m_protocol = string.Empty;
        private string m_readCodecName;
        private int m_readCodecRate;
        private string m_writeCodecName;
        private int m_writeCodecRate;

        /// <summary>
        /// Coded used to read information from the channel.
        /// Example: L16
        /// </summary>
        public string ReadCodecName
        {
            get { return m_readCodecName; }
            set { m_readCodecName = value; }
        }

        /// <summary>
        /// Bitrate of the read coded that is used.
        /// Example: 8000
        /// </summary>
        public int ReadCodecRate
        {
            get { return m_readCodecRate; }
            set { m_readCodecRate = value; }
        }

        /// <summary>
        /// Codec used for writing to the channel.
        /// Example: L16
        /// </summary>
        public string WriteCodecName
        {
            get { return m_writeCodecName; }
            set { m_writeCodecName = value; }
        }

        /// <summary>
        /// Bitrate of the codec.
        /// Example: 8000
        /// </summary>
        public int WriteCodecRate
        {
            get { return m_writeCodecRate; }
            set { m_writeCodecRate = value; }
        }

        /// <summary>
        /// State that the channel is in. Check ChannelState enum for more information.
        /// </summary>
        public ChannelState State
        {
            get { return m_state; }
            set { m_state = value; }
        }

        /// <summary>
        /// Number of the state (from the enum in FreeSwitch)
        /// </summary>
        public int StateNumber
        {
            get { return m_stateNumber; }
            set { m_stateNumber = value; }
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
            get { return m_protocol + "/" + _profile + "/" + m_address + "@" + _hostName; }
            set 
            { 
                string[] bits = value.Split('/');
                if (bits.Length == 3)
                {
                    m_protocol = bits[0];
                    _profile = bits[1];
                    string[] userParts = bits[2].Split('@');
                    if (userParts.Length == 2)
                    {
                        m_address = userParts[0];
                        HostName = userParts[1];
                    }
                    else
                        m_address = bits[2];
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
            get { return m_address; }
            set { m_address = value; }
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
            get { return m_protocol;  }
        }

        public bool Parse(string name, string value)
        {
            switch (name)
            {
                case "channel-state":
                    m_state = StateFromString(value);
                    break;
                case "channel-state-number":
                    int.TryParse(value, out m_stateNumber);
                    break;
                case "channel-name":
                    Name = value;
                    break;
                case "channel-read-codec-name":
                    m_readCodecName = value;
                    break;
                case "channel-read-codec-rate":
                    int.TryParse(value, out m_readCodecRate);
                    break;
                case "channel-write-codec-name":
                    m_writeCodecName = value;
                    break;
                case "channel-write-codec-rate":
                    int.TryParse(value, out m_writeCodecRate);
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
                case "CS_CONSUME_MEDIA": return ChannelState.ConsumeMedia;
                case "CS_HIBERNATE": return ChannelState.Hibernate;
                case "CS_HANGUP": return ChannelState.Hangup;
                case "CS_DONE": return ChannelState.Done;
                default: return ChannelState.Unknown;
            }
        }

        public string StateToString(ChannelState state)
        {
            return "Unknown";
        }
    }
}
