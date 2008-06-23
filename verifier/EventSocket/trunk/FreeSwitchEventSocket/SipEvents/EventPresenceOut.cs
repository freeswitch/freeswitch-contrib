namespace FreeSwitch.EventSocket
{
    public class EventPresenceOut : EventPresence
    {
        public override string ToString()
        {
            return "PresenceOut." + base.ToString();
        }


    }
}
