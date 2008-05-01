using System;
using System.Collections.Generic;
using System.Text;

namespace FreeSwitch.EventSocket
{
    public class EventMessageQuery : EventBase
    {
        private string _account;

        public string Account
        {
            get { return _account; }
            set { _account = value; }
        }

        public override bool ParseCommand(string name, string value)
        {
            if (name == "message-account")
                Account = value;
            else
                return base.ParseCommand(name, value);

            return true;
        }
    }
}
