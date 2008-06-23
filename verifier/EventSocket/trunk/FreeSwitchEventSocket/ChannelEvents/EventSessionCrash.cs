namespace FreeSwitch.EventSocket.ChannelEvents
{
    public class EventSessionCrash : ChannelEvent
    {
        public override string ToString()
        {
            return "SessionCrash." + base.ToString();
        }
    }
}
