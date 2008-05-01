namespace FreeSwitch.EventSocket.Commands
{
    public class AnyCommand : Api
    {
        private readonly string _name;
        private readonly string _value;

        public AnyCommand(string name, string arguments)
        {
            _name = name;
            _value = arguments;
        }

        public override string Command
        {
            get { return _name; }
        }

        public override string Arguments
        {
            get { return _value; }
        }

        public override CommandReply CreateReply(string dataToParse)
        {
            if (dataToParse.Length > 0 && (dataToParse[0] == '+') || dataToParse == "OK")
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
