namespace FreeSwitch.EventSocket
{
    public class EventMessageWaiting : EventBase
    {
        private string _account;
        private int _newMessages;
        private int _newUrgentMessages;
        private int _totalMessages;
        private int _totalUrgentMessages;

        public EventMessageWaiting()
        {
            
        }

        public EventMessageWaiting(string account, int newMessages, int totalMessages)
        {
            _account = account;
            _newMessages = newMessages;
            _totalMessages = totalMessages;
        }

        /// <summary>
        /// Messages are for this account
        /// </summary>
        public string Account
        {
            get { return _account; }
            set { _account = value; }
        }

        /// <summary>
        /// Total count of messages
        /// </summary>
        public bool MessagesWaiting
        {
            get { return _newMessages != 0 || _newUrgentMessages != 0; }
        }

        /// <summary>
        /// Number of new messages
        /// </summary>
        public int NewMessages
        {
            get { return _newMessages; }
            set { _newMessages = value; }
        }

        /// <summary>
        /// Number of new urgent messages
        /// </summary>
        public int NewUrgentMessages
        {
            get { return _newUrgentMessages; }
            set { _newUrgentMessages = value; }
        }

        /// <summary>
        /// Total number of new/saved/old messages
        /// </summary>
        public int TotalMessages
        {
            get { return _totalMessages; }
            set { _totalMessages = value; }
        }

        /// <summary>
        /// Total number of new urgent messages
        /// </summary>
        public int TotalUrgentMessages
        {
            get { return _totalUrgentMessages; }
            set { _totalUrgentMessages = value; }
        }

        public override bool ParseCommand(string name, string value)
        {
            switch (name)
            {
                case "mwi-messages-waiting":
                    // are calculated by using voice messages
                    break;
                case "mwi-message-account":
                    _account = value;
                    break;
                case "mwi-voice-message":
                    return ParseVoiceMessages(value);
                default:
                    return base.ParseCommand(name, value);
            }

            return true;
        }

        private bool ParseVoiceMessages(string value)
        {
            /*
             switch_event_add_header(event, SWITCH_STACK_BOTTOM, "MWI-Voice-Message", "%d/%d (%d/%d)", 
				total_new_messages, total_saved_messages, total_new_urgent_messages, total_saved_urgent_messages);
             * */
            int pos = value.IndexOf('/');
            if (pos == -1)
                return false;
            if (!int.TryParse(value.Substring(0, pos), out _newMessages))
                return false;

            int oldPos = pos + 1;
            pos = value.IndexOf(' ', pos);
            if (pos == -1)
                return false;
            if (!int.TryParse(value.Substring(oldPos, pos - oldPos), out _totalMessages))
                return false;

            oldPos = pos + 2;
            pos = value.IndexOf('/', pos);
            if (pos == -1)
                return false;
            if (!int.TryParse(value.Substring(oldPos, pos - oldPos), out _newUrgentMessages))
                return false;

            oldPos = pos + 1;
            pos = value.IndexOf(')', pos);
            if (pos == -1)
                return false;
            if (!int.TryParse(value.Substring(oldPos, pos - oldPos), out _totalUrgentMessages))
                return false;

            return true;
        }

        public override string ToString()
        {
            string str = base.ToString();
            str += "WMI-Messages-Waiting: " + (MessagesWaiting ? "yes" : "no") + "\n";
            str += "WMI-Message-Account: " + _account + "\n";
            if (_newUrgentMessages != 0 || _newMessages != 0)
                str += string.Format("WMI-Voice-Message: {0}/{1} ({2}/{3})\n",
                                     _newMessages,
                                     _totalMessages,
                                     _newMessages,
                                     _totalUrgentMessages);
            return str;
        }
    }
}