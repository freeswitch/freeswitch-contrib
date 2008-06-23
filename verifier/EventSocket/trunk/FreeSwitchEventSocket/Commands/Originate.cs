using System.Collections.Generic;
using FreeSwitch.EventSocket.General;

namespace FreeSwitch.EventSocket.Commands
{
    public class OriginateReply : CommandReply
    {
        private readonly string _sessionId;

        public OriginateReply(bool success, string sessionId) : base(success)
        {
            _sessionId = sessionId;
        }

        public string SessionId
        {
            get { return _sessionId; }
        }
    }

    public class Originate : Api
    {
        private SofiaSipAddress _caller;
        private Address _destination;
        private readonly IList<ChannelVariable> _variables = new List<ChannelVariable>();
        private string _callerIdName = null;
        private string _callerIdNumber = null;
        private bool _varsAdded = false;

        public Originate()
        {}

        public Originate(SofiaSipAddress caller, Address destination)
        {
            _caller = caller;
            _destination = destination;
        }

        public SofiaSipAddress Caller
        {
            get { return _caller; }
            set { _caller = value; }
        }

        public Address Destination
        {
            get { return _destination; }
            set { _destination = value; }
        }

        public IList<ChannelVariable> Variables
        {
            get { return _variables; }
        }

        public string CallerIdNumber
        {
            get { return _callerIdNumber; }
            set { _callerIdNumber = value; }
        }

        public string CallerIdName
        {
            get { return _callerIdName; }
            set { _callerIdName = value; }
        }

        public override string Command
        {
            get { return "originate"; }
        }

        public override string Arguments
        {
            get
            {
                if (!_varsAdded)
                {
                    _variables.Add(new ChannelVariable("origination_caller_id_name", CallerIdName ?? _caller.Extension));
                    if (!string.IsNullOrEmpty(_callerIdNumber))
                        _variables.Add(new ChannelVariable("origination_caller_id_number", _callerIdNumber));
                    _varsAdded = true;
                }

                string variables = string.Empty;
                foreach (ChannelVariable var in _variables)
                    variables += var + ",";
                if (variables.Length > 0)
                    variables = "{" + variables.Remove(variables.Length - 1, 1) + "}";

                return variables + Caller + " " + Destination;
            }
        }

        public override CommandReply CreateReply(string dataToParse)
        {
            string[] nameValue = dataToParse.Split(' ');
            if (nameValue[0].Length > 0 && nameValue[0][0] == '+')
                return new OriginateReply(true, nameValue[1]);
            else
            {
                OriginateReply reply = new OriginateReply(false, string.Empty);
                if (nameValue.Length > 1)
                    reply.ErrorMessage = nameValue[1];
                else
                    reply.ErrorMessage = dataToParse;
                return reply;
            }
        }

    }
}
