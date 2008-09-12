using System;
using System.Collections.Generic;
using System.Text;
using FreeSwitch.EventSocket;
using FreeSwitch.EventSocket.Commands;
using FreeSwitch.EventSocket.General;

namespace FreeSwitch.EventSocket.Test
{
    class EventSocket
    {
        private FreeSwitch.EventSocket.EventSocket m_es = new FreeSwitch.EventSocket.EventSocket(null);

        public void Setup()
        {
            m_es.Connect("192.168.1.102");
            m_es.Subscribe(new Events(Event.All));
            //m_es.SendApi(new Originate(new SofiaSipAddress("gauffin.com", "arne"), new Address("8888")));
            System.Threading.Thread.Sleep(500000);
        }
    }
}
