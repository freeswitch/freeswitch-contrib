using System;
using System.Collections.Generic;
using System.IO;
using System.Reflection;
using System.Text;

namespace FreeSwitch.EventSocket.Test
{
    class Program
    {
        static void Main(string[] args)
        {
            EventManager mgr = new EventManager();
            mgr.Subscribe(Events.GetChannelEvents());
            mgr.Start("localhost");
            Console.ReadLine();

            string buffer = File.ReadAllText("..\\..\\..\\watcherRaw.log");
            int bufLen = buffer.Length;

            EventParser ep = new EventParser(buffer);
            //int cnt = 0;
            EventSocket es = new EventSocket();
            es.Setup();
        }
    }
}
