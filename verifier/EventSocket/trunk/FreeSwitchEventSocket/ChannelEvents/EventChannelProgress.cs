namespace FreeSwitch.EventSocket
{
    public class EventChannelProgress : EventChannelState
    {
        public override string ToString()
        {
            return "ChannelProgress" + base.ToString();
        }
    }
}
