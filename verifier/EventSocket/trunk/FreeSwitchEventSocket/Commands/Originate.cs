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
        private readonly IList<CallVariable> _variables = new List<CallVariable>();
        private string _callerIdName;
        private string _callerIdNumber;
        private bool _varsAdded;
        private bool _autoAnswer;

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

        public IList<CallVariable> Variables
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
                    string name = CallerIdName ?? _caller.Extension;
                    _variables.Add(new CallVariable("origination_caller_id_name", "'" + name + "'"));
                    if (!string.IsNullOrEmpty(_callerIdNumber))
                        _variables.Add(new CallVariable("origination_caller_id_number", _callerIdNumber));
                    _varsAdded = true;

                    if (_autoAnswer)
                    {
                        _variables.Add(new CallLegVariable("sip_invite_params", "intercom=true"));
                        _variables.Add(new CallLegVariable("sip_h_Call-Info", "<sip:$${domain}>;answer-after=0"));
                        _variables.Add(new CallLegVariable("sip_auto_answer", "true"));
                    }
                }

                string variables = string.Empty;
                string legVariables = string.Empty;
                foreach (CallVariable var in _variables)
                {
                    if (var is CallLegVariable)
                        legVariables += var + ",";
                    else
                        variables += var + ",";
                }
                if (variables.Length > 0)
                    variables = "{" + variables.Remove(variables.Length - 1, 1) + "}";
                if (legVariables.Length > 0)
                    legVariables = "[" + legVariables.Remove(legVariables.Length - 1, 1) + "]";

                return variables + legVariables + Caller + " " + Destination;
            }
        }

        public bool AutoAnswer
        {
            get { return _autoAnswer; }
            set { _autoAnswer = value; }
        }

        public override CommandReply CreateReply(string dataToParse)
        {
            string[] nameValue = dataToParse.Split(' ');
            if (nameValue[0].Length > 0 && nameValue[0][0] == '+')
                return new OriginateReply(true, nameValue[1]);
            
            OriginateReply reply = new OriginateReply(false, string.Empty);
            reply.ErrorMessage = nameValue.Length > 1 ? nameValue[1] : dataToParse;
            return reply;
        }

    }
}
