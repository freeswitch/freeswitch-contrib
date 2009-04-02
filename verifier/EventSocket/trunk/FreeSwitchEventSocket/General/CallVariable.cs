namespace FreeSwitch.EventSocket.General
{
    /// <summary>
    /// Variable that applies to both legs in a call.
    /// </summary>
    /// <remarks>
    /// Variables can be access from FreeSWITCH with the name
    /// "variable_[name]"
    /// </remarks>
    public class CallVariable
    {
        private readonly string _name;
        private readonly string _value;

        /// <summary>
        /// Initializes a new instance of the <see cref="CallVariable"/> class.
        /// </summary>
        /// <param name="name">Name of variable.</param>
        /// <param name="value">Value.</param>
        public CallVariable(string name, string value)
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
