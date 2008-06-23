namespace FreeSwitch.EventSocket
{
    public class EventPresenceIn : EventPresence
    {
        public override string ToString()
        {
            return "PresenceIn." + base.ToString();
        }
    }
}
