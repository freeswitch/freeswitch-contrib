namespace FreeSwitch.EventSocket.Commands
{
    public class ParkCmd : Api
    {
        private readonly string _uuid;

        public ParkCmd(string uuid)
        {
            _uuid = uuid;    
        }

        public override string Command
        {
            get { return "uuid_park"; }
        }

        public override string Arguments
        {
            get { return _uuid; }
        }
    }
}
