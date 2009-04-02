namespace FreeSwitch.EventSocket.Commands
{
    public class HoldCmd : Api
    {
        private readonly string _sessionId;
        private readonly bool _activate;

        public HoldCmd(string sessionId, bool activate)
        {
            _sessionId = sessionId;
            _activate = activate;
        }

        public override string Command
        {
            get { return "uuid_hold"; }
        }

        public override string Arguments
        {
            get { return (_activate ? string.Empty : "off ") + _sessionId; }
        }
    }
}
