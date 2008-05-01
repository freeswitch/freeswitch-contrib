using System;
using System.Collections.Generic;
using System.Text;

namespace FreeSwitch.EventSocket.Commands
{
    public class GetVariable : Api
    {
        private readonly string _channelId;
        private readonly string _name;

        public GetVariable()
        {
            
        }

        public GetVariable(string uuid, string name)
        {
            _channelId = uuid;
            _name = name;
        }

        public override string Command
        {
            get { return "uuid_getvar"; }
        }

        public override string Arguments
        {
            get { return _channelId + " " + _name; }
        }

        public override CommandReply CreateReply(string dataToParse)
        {
            if (dataToParse.Substring(0, 2) == "OK")
            {
                return new GetVariableReply(true, dataToParse.Substring(3));
            }
            else
            {
                GetVariableReply reply = new GetVariableReply(false, string.Empty);
                reply.ErrCode = dataToParse;
                return reply;
            }
        }
    }


    public class GetVariableReply : CommandReply
    {
        private string _value;

        public GetVariableReply(bool success, string value)
            : base(success)
        {
            _value = value;
        }

        public string SessionId
        {
            get { return _value; }
        }

        public string Value
        {
            get { return _value; }
        }
    }
}
