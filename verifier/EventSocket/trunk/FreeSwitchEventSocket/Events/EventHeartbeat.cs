namespace FreeSwitch.EventSocket
{
    class EventHeartbeat : EventBase
    {
        private string _info;
        private string _upTime;
        private int _sessionCount;

        public string Info
        {
            get { return _info; }
            set { _info = value; }
        }

        public string UpTime
        {
            get { return _upTime; }
            set { _upTime = value; }
        }

        public int SessionCount
        {
            get { return _sessionCount; }
            set { _sessionCount = value; }
        }

        public override bool ParseCommand(string name, string value)
        {
            switch (name)
            {
                case "event-info":
                    Info = value;
                    break;
                case "up-time":
                    UpTime = value;
                    break;
                case "session-count":
                    int.TryParse(value, out _sessionCount);
                    break;

                default:
                    return base.ParseCommand(name, value);
            }

            return true;

        }
    }

/*
 Event-Info: System%20Ready
Up-Time: 0%20years,%200%20days,%201%20hour,%2048%20minutes,%207%20seconds,%20265
%20milliseconds,%20625%20microseconds%0A
Session-Count: 0
Event-Name: HEARTBEAT
 */
}
