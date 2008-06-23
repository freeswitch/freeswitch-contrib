namespace FreeSwitch.EventSocket
{
    public class ChannelEvent : EventBase
    {
        private ChannelInfo _channelInfo;
        private string _uniqueId = string.Empty;
        private string _answerState;
        private string _callDirection;

        public override bool IsChannelEvent
        {
            get { return true; }
        }

        /// <summary>
        /// Information about the used channel and it's state.
        /// </summary>
        public ChannelInfo ChannelInfo
        {
            get { return _channelInfo; }
            set { _channelInfo = value; }
        }

        public string UniqueId
        {
            get { return _uniqueId; }
            set { _uniqueId = value; }
        }

        public string AnswerState
        {
            get { return _answerState; }
            set { _answerState = value; }
        }

        public string CallDirection
        {
            get { return _callDirection; }
            set { _callDirection = value; }
        }

        public override bool ParseCommand(string name, string value)
        {
            if (name == "unique-id")
                _uniqueId = value;
            else if (name == "answer-state")
                _answerState = value;
            else if (name == "call-direction")
                _callDirection = value;
            else if (name.Length > 8 && name.Substring(0, 8) ==  "channel-")
            {
                if (_channelInfo == null)
                    _channelInfo = new ChannelInfo();
                return _channelInfo.Parse(name, value);
            }
            else
                return base.ParseCommand(name, value);

            return true;
        }

        public override string ToString()
        {
            return
                "ChannelEvent(" + _uniqueId + " [" + _callDirection + "] " + _answerState +
                ", channelInfo{" + _channelInfo + "})";
        }
    }
}
