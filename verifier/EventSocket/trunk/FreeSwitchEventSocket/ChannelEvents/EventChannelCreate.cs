namespace FreeSwitch.EventSocket
{
    public class EventChannelCreate : ChannelEvent
    {
        public override string ToString()
        {
            return "ChannelCreate." + base.ToString();
        }
    }
}
