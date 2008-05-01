namespace FreeSwitch.EventSocket.Commands
{
    public abstract class SendMsg : CmdBase
    {
        private readonly string _uuid;
        private readonly string _callCommand = "execute";

        public SendMsg(string uuid)
        {
            _uuid = uuid;
        }

        public SendMsg(string uuid, string callCommand)
        {
            _callCommand = callCommand;
            _uuid = uuid;
        }

        public override string ToString()
        {
            return
                string.Format("SendMsg {0}\ncall-command: {1}\nexecute-app-name: {2}\nexecute-app-arg: {3}\n", _uuid,
                              _callCommand, Command, Arguments);
        }
    }
}
