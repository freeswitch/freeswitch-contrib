namespace FreeSwitch.EventSocket
{
    public class EventChannelExecute : EventChannelState
    {
        private string _appName;
        private string _arguments;

        public string AppName
        {
            get { return _appName; }
            set { _appName = value; }
        }

        public string Arguments
        {
            get { return _arguments; }
            set { _arguments = value; }
        }

        public override bool ParseCommand(string name, string value)
        {
            if (name == "application")
                AppName = value;
            else if (name == "application-data")
                Arguments = value;
            else
                return base.ParseCommand(name, value);

            return true;
        }

        public override string ToString()
        {
            return "EventChannelExecute(" + _appName + ", '" + _arguments + "')." + base.ToString();
        }
    }
}
