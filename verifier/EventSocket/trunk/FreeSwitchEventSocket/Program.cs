using System;
using System.Collections.Generic;
using System.Text;
using FreeSwitch.EventSocket;

namespace FreeSwitch.EventSocket
{
    class Program
    {
        /// <summary>
        /// This method is called by the event delegate.
        /// </summary>
        /// <param name="eb">Reference to an event, each event has their own class.</param>
        void HandleEvent(EventBase eb)
        {
            //Console.WriteLine(eb.DateLocal + "  Got Event: " + eb.Name);


            if (eb is EventChannelHangup)
                Console.WriteLine(eb.DateLocal + " Hangup reason: " + ((EventChannelHangup)eb).Cause.ToString());
        }

        static void Main(string[] args)
        {
            Program p = new Program();

            /// Create a new FreeSwitch event socket.
            EventManager eventManager = new EventManager();

            // Hook all events to the same delegate. Can also provide one delegate per event.
            eventManager.EventReceived += p.HandleEvent;
            eventManager.Subscribe(new Events(Event.All));

            // Connect to localhost
            eventManager.Start("localhost");

            
            System.Threading.Thread.Sleep(1000000);
            
            //a.
        }
    }
}
