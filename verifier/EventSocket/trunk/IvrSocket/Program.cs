using System;
using System.Threading;
using FreeSwitch.EventSocket;
using FreeSwitch.EventSocket.Commands;

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
            Thread.Sleep(1000);
            //mgr.Send(new SendEventCmd(new EventMessageWaiting("jonas@192.168.0.58", 1, 2)));
            Console.ReadLine();
        }

        private static void IvrManager(EventBase receivedEvent)
        {
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
