namespace FreeSwitch.EventSocket.Commands
{
    #region Delegates

    public delegate void ReplyHandler(CmdBase command, CommandReply reply);

    #endregion

    public abstract class CmdBase
    {
        /// <summary>
        /// Node that the reply handler is removed after it have been invoked.
        /// This is to make sure that the command can be garbage collected when it's done.
        /// </summary>
        public event ReplyHandler ReplyReceived;

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
            if (ReplyReceived != null)
            {
                ReplyReceived(this, reply);
                ReplyReceived = null;                
            }
        }

        public virtual CommandReply CreateReply(string dataToParse)
        {
            if (string.IsNullOrEmpty(dataToParse.Trim()))
                return new CommandReply(false);

            CommandReply reply = new CommandReply(dataToParse.Trim()[0] == '+');
            reply.ErrorMessage = dataToParse;
            return reply;
        }

        public override string ToString()
        {
            return string.Format("{0} {1}", Command, Arguments);
        }
    }
}
