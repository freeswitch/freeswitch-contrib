namespace FreeSwitch.EventSocket.Commands
{
    public class AuthCommand : CmdBase
    {
        private readonly string _password;

        public AuthCommand(string password)
        {
            _password = password;
        }

        public override string Command
        {
            get { return "auth"; }
        }

        public override string Arguments
        {
            get { return _password; }
        }

        public override CommandReply CreateReply(string dataToParse)
        {
            if (dataToParse.Contains("+OK"))
                return new CommandReply(true);
            else
            {
                CommandReply reply = new CommandReply(false);
                reply.ErrCode = dataToParse;
                return reply;
            }
        }
    }
}