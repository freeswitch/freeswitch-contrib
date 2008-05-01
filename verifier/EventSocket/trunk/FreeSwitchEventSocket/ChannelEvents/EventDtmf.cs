namespace FreeSwitch.EventSocket
{
    public class EventDtmf : ChannelEvent
    {
        private char _digit;
        private int _duration;

        public char Digit
        {
            get { return _digit; }
            set { _digit = value; }
        }

        public int Duration
        {
            get { return _duration; }
            set { _duration = value; }
        }

        public override bool ParseCommand(string name, string value)
        {
            switch(name)
            {
                case "dtmf-digit":
                    _digit = value[0];
                    break;
                case "dtmf-duration":
                    int.TryParse(value, out _duration);
                    break;
            }

            return base.ParseCommand(name, value);
        }
    }
}
