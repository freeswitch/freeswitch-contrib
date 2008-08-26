namespace FreeSwitch.EventSocket
{
    public class EventPresenceIn : EventPresence
    {
        public override string ToString()
        {
            return "In." + base.ToString();
        }
    }
}
