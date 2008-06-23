namespace FreeSwitch.EventSocket
{
    /// <summary>
    /// Reports when DTMF digits have been pressed
    /// </summary>
    public class EventDtmfStatus : ChannelEvent
    {
        private int _count;

        /// <summary>
        /// Number of digits in the buffer.
        /// </summary>
        public int Count
        {
            get { return _count; }
        }


        public override bool ParseCommand(string name, string value)
        {
            switch (name)
            {
                case "dtmf-count":
                    int.TryParse(value, out _count);
                    break;
            }

            return base.ParseCommand(name, value);
        }

        public override string ToString()
        {
            return "DtmfStatus." + base.ToString();
        }
    }
}
