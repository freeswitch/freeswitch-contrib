namespace FreeSwitch.EventSocket.General
{
    /// <summary>
    /// Channel variables are used in freeswitch to store information
    /// that can be used by javascript programs.
    /// </summary>
    public class ChannelVariable
    {
        private readonly string _name;
        private readonly string _value;

        public ChannelVariable(string name, string value)
        {
            _name = name;
            _value = value;
        }

        public string Name
        {
            get { return _name; }
        }

        public string Value
        {
            get { return _value; }
        }

        public override string ToString()
        {
            return _name + "=" + _value;
        }
    }
}
