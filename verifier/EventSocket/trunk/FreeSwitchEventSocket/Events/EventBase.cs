using System;
using System.Collections.Specialized;

namespace FreeSwitch.EventSocket
{
    public class EventBase
    {
        private string m_callingFile;
        private string m_callingFunction;
        private int m_callingLineNumber;
        private string m_coreId;
        private DateTime m_dateGMT;
        private DateTime m_dateLocal;
        private string m_name;
        private readonly NameValueCollection _parameters = new NameValueCollection();
        private NameValueCollection _allParameters;

        #region Properties

        public string Name
        {
            get
            {
                lock (this)
                {
                    if (string.IsNullOrEmpty(m_name))
                    {
                        string name = GetType().Name.Substring(5); //remove (Event)
                        m_name = string.Empty;
                        m_name += char.ToUpper(name[0]);
                        for (int i = 1; i < name.Length; ++i)
                        {
                            if (char.IsUpper(name[i]))
                                m_name += '_';
                            else
                                m_name += char.ToUpper(name[i]);
                        }
                    }
                }

                return m_name;
            }
            set { m_name = value; }
        }

        public string CoreId
        {
            get { return m_coreId; }
            set { m_coreId = value; }
        }

        public DateTime DateLocal
        {
            get { return m_dateLocal; }
            set { m_dateLocal = value; }
        }

        public DateTime DateGMT
        {
            get { return m_dateGMT; }
            set { m_dateGMT = value; }
        }

        public string CallingFile
        {
            get { return m_callingFile; }
            set { m_callingFile = value; }
        }

        public string CallingFunction
        {
            get { return m_callingFunction; }
            set { m_callingFunction = value; }
        }

        public int CallingLineNumber
        {
            get { return m_callingLineNumber; }
            set { m_callingLineNumber = value; }
        }

        #endregion

        public virtual bool IsChannelEvent
        {
            get { return false; }
        }

        public NameValueCollection Parameters
        {
            get { return _parameters; }
        }

        public NameValueCollection AllParameters
        {
            get { return _allParameters; }
        }

        internal void SetParameters(NameValueCollection allparams)
        {
            _allParameters = allparams;
        }

        public void Parse(NameValueCollection variables)
        {
            for (int i = 0; i < variables.Count; ++i)
                ParseCommand(variables.GetKey(i), variables.Get(i));
        }

        public static EventBase CreateEvent(string eventName)
        {
            try
            {
                string eventNamea = "Event" + eventName;
                Type t = Type.GetType("FreeSwitch.EventSocket.Event" + eventName);
                if (t == null)
                    return null;
                return (EventBase) Activator.CreateInstance(t);
            }
            catch (TypeLoadException)
            {
                return null;
            }
        }

        public virtual bool ParseCommand(string name, string value)
        {
            switch (name)
            {
                case "event-name":
                    m_name = value;
                    break;
                case "core-uuid":
                    m_coreId = value;
                    break;
                case "event-date-local":
                    m_dateLocal = DateTime.Parse(value);
                    break;
                case "event-date-gmt":
                    m_dateGMT = DateTime.Parse(value);
                    break;
                case "event-calling-file":
                    m_callingFile = value;
                    break;
                case "event-calling-function":
                    m_callingFunction = value;
                    break;
                case "event-calling-line-number":
                    int.TryParse(value, out m_callingLineNumber);
                    break;
                default:
                    Parameters.Add(name, value);
                    //Console.WriteLine(m_name + ", Unhandled parameter: [" + name + "] " + value);
                    return false;
            }
            return true;
        }

        public override string ToString()
        {
            return "Base(" + m_name + ")";
        }
    }
}