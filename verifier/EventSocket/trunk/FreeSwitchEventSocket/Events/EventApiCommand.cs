using System;
using System.Collections.Generic;
using System.Text;

namespace FreeSwitch.EventSocket
{
    class EventApiCommand : EventBase
    {
        private string m_command;

        public override bool ParseCommand(string name, string value)
        {
            switch (name)
            {
                case "api-command":
                    m_command = value;
                    break;
                default:
                    return base.ParseCommand(name, value);
            }

            return true;
        }
    }
}
