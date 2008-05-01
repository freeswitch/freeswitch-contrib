namespace FreeSwitch.EventSocket.Commands
{
    public abstract class Api : CmdBase
    {
        public override string ToString()
        {
            return string.Format("api {0} {1}", Command, Arguments);
        }
    }
}
