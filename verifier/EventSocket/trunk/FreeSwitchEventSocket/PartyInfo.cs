namespace FreeSwitch.EventSocket
{
    public class PartyInfo
    {
        public static PartyInfo Empty = new PartyInfo();
        private string _userName = string.Empty;
        private string m_dialplan = string.Empty;
        private string m_callerIdName = string.Empty;
        private string m_callerIdNumber = string.Empty;
        private string m_destinationNumber = string.Empty;
        private string m_uniqueId = string.Empty;
        private string m_source = string.Empty;
        private string m_context = string.Empty;
        private string _channelName = string.Empty;
        private bool m_screenBit;
        private bool m_privacyHideName;
        private bool m_privacyHideNumber;

        public PartyInfo()
        {
            _userName = string.Empty;
            m_dialplan = string.Empty;
            m_callerIdName = string.Empty;
        }

        public string Dialplan
        {
            get { return m_dialplan; }
            set { m_dialplan = value; }
        }

        public string CallerIdName
        {
            get { return m_callerIdName; }
            set { m_callerIdName = value; }
        }

        public string CallerIdNumber
        {
            get { return m_callerIdNumber; }
            set { m_callerIdNumber = value; }
        }

        public string DestinationNumber
        {
            get { return m_destinationNumber; }
            set { m_destinationNumber = value; }
        }

        public string UniqueId
        {
            get { return m_uniqueId; }
            set { m_uniqueId = value; }
        }

        public string Source
        {
            get { return m_source; }
            set { m_source = value; }
        }

        public string Context
        {
            get { return m_context; }
            set { m_context = value; }
        }

        public string ChannelName
        {
            get { return _channelName; }
            set { _channelName = value; }
        }

        public bool ScreenBit
        {
            get { return m_screenBit; }
            set { m_screenBit = value; }
        }

        public bool PrivacyHideName
        {
            get { return m_privacyHideName; }
            set { m_privacyHideName = value; }
        }

        public bool PrivacyHideNumber
        {
            get { return m_privacyHideNumber; }
            set { m_privacyHideNumber = value; }
        }

        public string UserName
        {
            get { return _userName; }
            set { _userName = value; }
        }

        public bool Parse(string name, string value)
        {
            switch (name)
            {
                case "username":
                    _userName = value;
                    break;
                case "dialplan":
                    Dialplan = value;
                    break;
                case "caller-id-name":
                    CallerIdName = value;
                    break;
                case "caller-id-number":
                    CallerIdNumber = value;
                    break;
                case "destination-number":
                    DestinationNumber = value;
                    break;
                case "unique-id":
                    UniqueId = value;
                    break;
                case "source":
                    Source = value;
                    break;
                case "context":
                    Context = value;
                    break;
                case "channel-name":
                    ChannelName = value;
                    break;
                case "screen-bit":
                    ScreenBit = value == "yes";
                    break;
                case "privacy-hide-name":
                    PrivacyHideName = value == "yes";
                    break;
                case "privacy-hide-number":
                    PrivacyHideNumber = value == "yes";
                    break;
                default:
                    return false;
            }

            return true;

        }

        public override string ToString()
        {
            return _channelName + "(" + CallerIdName + "/" + CallerIdNumber + ") id: "+UniqueId+", destination: " + DestinationNumber;
            
        }

    }
}
