namespace FreeSwitch.EventSocket.Commands
{
    #region Delegates

    public delegate void ReplyHandler(CmdBase command, CommandReply reply);

    #endregion

    public abstract class CmdBase
    {
        public event ReplyHandler OnReply;

        private object _contextData;

        public abstract string Command
        {
            get;
        }

        public abstract string Arguments
        {
            get;
        }

        /// <summary>
        /// You can attach something that you need to
        /// identify the command.
        /// </summary>
        public object ContextData
        {
            get { return _contextData; }
            set { _contextData = value; }
        }

        public virtual void HandleReply(CommandReply reply)
        {
            // a command is only invoked once.
            if (OnReply != null)
            {
                OnReply(this, reply);
                OnReply = null;                
            }
        }

        public virtual CommandReply CreateReply(string dataToParse)
        {
            return null;
        }

        public override string ToString()
        {
            return string.Format("{0} {1}", Command, Arguments);
        }
    }
}
