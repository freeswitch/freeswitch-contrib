using System;
using System.Collections.Generic;
using System.Text;

namespace FreeSwitch.EventSocket.Commands
{
    public class SendEventCmd : CmdBase
    {
        private EventBase _event;

        /// <summary>
        /// 
        /// </summary>
        /// <param name="evt">Note that all events have NOT being developed to be sent, only to be received.</param>
        public SendEventCmd(EventBase evt)
    {
            _event = evt;
    }


        public override string Command
        {
            get { return "sendevent " + _event.Name + "\n"; }
        }

        public override string Arguments
        {
            get { return _event.ToString(); }
        }
    }
}
