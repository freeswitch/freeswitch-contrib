using System;

namespace FreeSwitch.EventSocket.Commands
{
    public class DeflectCmd : SendMsg
    {
        private readonly string _uri;

        /// <summary>
        /// 
        /// </summary>
        /// <param name="uuid">session id</param>
        /// <param name="sipUri">complete sip uri, with or whithout "sip:"</param>
        public DeflectCmd(string uuid, string sipUri)
            : base(uuid, "execute")
        {
            if (string.IsNullOrEmpty(sipUri))
                throw new ArgumentNullException("sipUri");

            _uri = sipUri;
        }

        public DeflectCmd(string uuid) : base(uuid)
        {
        }

        public override string Command
        {
            get { return "deflect"; }
        }

        public override string Arguments
        {
            get { return _uri; }
        }
    }
}
