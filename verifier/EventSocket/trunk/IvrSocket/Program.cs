using System;
using FreeSwitch.EventSocket;

namespace IvrSocket
{
    class Program
    {
        private static EventManager mgr;
        static void Main(string[] args)
        {
            mgr = new EventManager();
            mgr.EventReceived += IvrManager;
            mgr.Subscribe(Event.All);
            mgr.Start("localhost");
            Console.ReadLine();
        }

        private static void IvrManager(EventBase receivedEvent)
        {
            Console.WriteLine(receivedEvent.Name);
            if (receivedEvent is ChannelEvent)
            {
                ChannelEvent evt = (ChannelEvent) receivedEvent;
                Console.WriteLine(evt.Name + ": " + evt.UniqueId);
            }
            if (receivedEvent is EventChannelAnswer)
            {
                EventChannelAnswer answer = (EventChannelAnswer)receivedEvent;
                if (answer.Parameters["variable_ivr_name"] == "voicemail")
                   new Voicemail(mgr, answer.UniqueId);
            }
            else if (receivedEvent is EventChannelHangup)
            {
                Console.WriteLine("Bye bye baby!");
            }
        }
    }
}
