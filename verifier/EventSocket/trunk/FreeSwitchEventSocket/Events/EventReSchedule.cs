using System;
using System.Collections.Generic;
using System.Text;

namespace FreeSwitch.EventSocket
{
    class EventReSchedule : EventBase
    {
        private int m_taskId;
        private string m_description;
        private string m_group;
        private string m_runtime;

        public int TaskId
        {
            get { return m_taskId; }
            set { m_taskId = value; }
        }

        public string Description
        {
            get { return m_description; }
            set { m_description = value; }
        }

        public string Group
        {
            get { return m_group; }
            set { m_group = value; }
        }

        public string Runtime
        {
            get { return m_runtime; }
            set { m_runtime = value; }
        }

        public override bool ParseCommand(string name, string value)
        {
            switch (name)
            {
                case "task-id":
                    int.TryParse(value, out m_taskId);
                    break;
                case "task-desc":
                    m_description = value;
                    break;
                case "task-group":
                    m_group = value;
                    break;
                case "task-runtime":
                    m_runtime = value;
                    break;
                default:
                    return base.ParseCommand(name, value);
            }

            return true;

        }
    }
}
