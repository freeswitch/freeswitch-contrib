namespace FreeSwitch.EventSocket.Commands
{
    public class SleepCmd : SendMsg
    {
        private readonly int _ms;

        public SleepCmd(string uuid, int ms) : base(uuid)
        {
            _ms = ms;
        }

        public override string Command
        {
            get { return "sleep"; }
        }

        public override string Arguments
        {
            get { return _ms.ToString(); }
        }
    }
}
