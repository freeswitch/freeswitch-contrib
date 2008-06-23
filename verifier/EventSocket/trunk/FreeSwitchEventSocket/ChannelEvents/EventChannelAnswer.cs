namespace FreeSwitch.EventSocket
{
    public class EventChannelAnswer : EventChannelState
    {
        public override string ToString()
        {
            return "ChannelAnswer." + base.ToString();
        }
    }
}
