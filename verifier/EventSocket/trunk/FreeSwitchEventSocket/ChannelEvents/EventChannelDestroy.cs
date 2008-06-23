namespace FreeSwitch.EventSocket
{
    public class EventChannelDestroy : EventChannelState
    {
        public override string ToString()
        {
            return "ChannelDestroy." + base.ToString();
        }
    }
}
