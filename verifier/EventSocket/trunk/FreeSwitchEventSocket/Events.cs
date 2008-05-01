using System.Collections.Generic;

namespace FreeSwitch.EventSocket
{
    #region Event enum
    public enum Event
    {
        Custom,
        ChannelCreate,
        ChannelDestroy,
        ChannelState,
        ChannelAnswer,
        ChannelApplication,
        ChannelHangup,
        ChannelExecute,
        ChannelExecuteComplete,
        ChannelBridge,
        ChannelUnbridge,
        ChannelProgress,
        ChannelOriginate,
        ChannelOutgoing,
        ChannelPark,
        ChannelUnpark,
        Api,
        Log,
        InboundChan,
        OutboundChan,
        Startup,
        Shutdown,
        Publish,
        Unpublish,
        Talk,
        Notalk,
        SessionCrash,
        ModuleLoad,
        Dtmf,
        Message,
        PresenceIn,
        PresenceOut,
        PresenceProbe,
        SofiaRegister,
        SofiaExpires,
        Roster,
        Codec,
        BackgroundJob,
        DetectedSpeech,
        PrivateCommand,
        Heartbeat,
        All
    }
    #endregion

    #region Events class
    public class Events
    {
        private IList<Event> m_events = new List<Event>();

        public Events() { }
        public Events(params Event[] parameters)
        {
            foreach (Event e in parameters)
            {
                Add(e);
            }
        }

        public void Add(Event e)
        {
            m_events.Add(e);
        }

        public Event this[int index]
        {
            get { return m_events[index]; }
        }

        public void Clear()
        {
            m_events.Clear();
        }

        public int Count
        {
            get { return m_events.Count; }
        }

        public static Events GetChannelEvents()
        {
            Events e = new Events();
            e.Add(Event.ChannelAnswer);
            e.Add(Event.ChannelBridge);
            e.Add(Event.ChannelCreate);
            e.Add(Event.ChannelDestroy);
            e.Add(Event.ChannelExecute);
            e.Add(Event.ChannelHangup);
            e.Add(Event.ChannelOutgoing);
            e.Add(Event.ChannelPark);
            e.Add(Event.ChannelProgress);
            e.Add(Event.ChannelState);
            e.Add(Event.ChannelUnbridge);
            e.Add(Event.ChannelUnpark);
            return e;
        }
    }
    #endregion

}