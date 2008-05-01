namespace FreeSwitch.EventSocket.Commands
{
    public class SetVariable : Api
    {
        private readonly string _channelId;
        private readonly string _name;
        private readonly string _value;

        public SetVariable()
        {
            
        }

        public SetVariable(string uuid, string name, string value)
        {
            _channelId = uuid;
            _name = name;
            _value = value;
        }

        public override string Command
        {
            get { return "uuid_setvar"; }
        }

        public override string Arguments
        {
            get { return _channelId + " " + _name + " " + _value; }
        }

        public override CommandReply CreateReply(string dataToParse)
        {
            if (dataToParse == "OK")
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
