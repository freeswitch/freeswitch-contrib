namespace FreeSwitch.EventSocket.Commands
{
    public class TransferCmd : PbxCommand
    {
        private readonly string _sessionId;
        private readonly string _destination;

        /// <summary>
        /// Transfer a call to somewhere.
        /// </summary>
        /// <param name="sessionId"></param>
        /// <param name="destination"></param>
        public TransferCmd(string sessionId, string destination)
        {
            _sessionId = sessionId;
            _destination = destination;
        }

        public override PbxCommandReply CreateReply(string dataToParse)
        {
            if (dataToParse == "OK")
                return new PbxCommandReply(true);
            else
            {
                PbxCommandReply reply = new PbxCommandReply(false);
                reply.ErrCode = dataToParse;
                return reply;
            }
        }

        public override string ToString()
        {
            return "transfer " + _sessionId + " " + _destination;
        }
    }
}
