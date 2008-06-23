namespace FreeSwitch.EventSocket
{
    public class EventChannelOutgoing : EventChannelState
    {
        public override string ToString()
        {
            return "ChannelOutgoing." + base.ToString();
        }
    }
}
