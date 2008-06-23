namespace FreeSwitch.EventSocket
{
    public class EventCodec : EventChannelState
    {
        public override string ToString()
        {
            return "Codec." + base.ToString();
        }
    }
}
