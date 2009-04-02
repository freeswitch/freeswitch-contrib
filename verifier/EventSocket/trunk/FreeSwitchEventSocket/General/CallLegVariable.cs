namespace FreeSwitch.EventSocket.General
{
    /// <summary>
    /// Variable stored only for a specific leg.
    /// </summary>
    /// <remarks>
    /// Channel variables are used in FreeSWITCH to store information
    /// that can be used by JavaScript programs.
    /// </remarks>
    public class CallLegVariable : CallVariable
    {
        /// <summary>
        /// Initializes a new instance of the <see cref="CallLegVariable"/> class.
        /// </summary>
        /// <param name="name">The name.</param>
        /// <param name="value">The value.</param>
        public CallLegVariable(string name, string value) : base(name, value)
        {
        }

    }
}
