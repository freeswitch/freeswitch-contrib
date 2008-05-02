namespace FreeSwitch.EventSocket.Commands
{
    /// <summary>
    /// Baseclass for replies that commands get.
    /// </summary>
    public class CommandReply
    {
        private bool _success;
        private string _ErrorMessage;

        public CommandReply(bool success)
        {
            _success = success;
        }

        public bool Success
        {
            get { return _success; }
        }

        public string ErrorMessage
        {
            get { return _ErrorMessage; }
            set { _ErrorMessage = value; }
        }

        public override string ToString()
        {
            if (_success)
                return "Success";
            else
                return "Failed: " + _ErrorMessage;
        }
    }
}
