using System.Collections.Generic;

namespace FreeSwitch.EventSocket.Commands
{
    public class ExecuteJavascript : Api
    {
        private readonly string _scriptName;
        private readonly IList<string> _arguments = new List<string>();

        public ExecuteJavascript()
        {
            
        }

        public ExecuteJavascript(string scriptName)
        {
            _scriptName = scriptName;
        }

        /// <summary>
        /// Arguments to javascript
        /// </summary>
        /// <seealso cref="Arguments"/>
        public IList<string> Args
        {
            get { return _arguments; }
        }

        public override string Command
        {
            get { return "jsrun"; }
        }

        /// <summary>
        /// Args to the CmdBase api
        /// </summary>
        /// <seealso cref="Args"/>
        public override string Arguments
        {
            get { return _scriptName + " " + JoinArguments(); }
        }

        public override CommandReply CreateReply(string dataToParse)
        {
            if (dataToParse == "OK")
                return new CommandReply(true);
            else
            {
                CommandReply reply = new CommandReply(false);
                reply.ErrorMessage = dataToParse;
                return reply;
            }
        }

        private string JoinArguments()
        {
            string args = string.Empty;
            foreach (string arg in Args)
                args += arg + " ";
            return args;
        }
    }
}
