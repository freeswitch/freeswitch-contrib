namespace FreeSwitch.EventSocket.Commands
{
    /// <summary>
    /// Baseclass for replies that commands get.
    /// </summary>
    public class CommandReply
    {
        private bool _success;
        private string _errCode;

        public CommandReply(bool success)
        {
            _success = success;
        }

        public bool Success
        {
            get { return _success; }
        }

        public string ErrCode
        {
            get { return _errCode; }
            set { _errCode = value; }
        }

        public override string ToString()
        {
            if (_success)
                return "Success";
            else
                return "Failed: " + _errCode;
        }
    }
}
