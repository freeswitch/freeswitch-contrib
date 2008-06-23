namespace FreeSwitch.EventSocket
{
    public class EventChannelOriginate : ChannelEvent
    {
        public override string ToString()
        {
            return "ChannelOriginate." + base.ToString();
        }
    }
}
