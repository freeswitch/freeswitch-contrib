using System;
using System.Collections.Generic;
using System.Text;

namespace FreeSwitch.EventSocket.SipEvents
{
    class EventPresenceProbe : EventPresence
    {
        public override string ToString()
        {
            return "PresenceProbe." + base.ToString();
        }

    }

}
