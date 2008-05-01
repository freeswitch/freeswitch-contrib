namespace FreeSwitch.EventSocket.Commands
{
    /// <summary>
    /// Play a file
    /// </summary>
    public class PlaybackCmd : SendMsg
    {
        private readonly string _fileName;

        /// <summary>
        /// 
        /// </summary>
        /// <param name="uuid">channel identifier</param>
        /// <param name="fileName">Path and filename to sound file</param>
        /// <remarks>Path should be relative to FS path.</remarks>
        /// <example>
        /// mgr.SendApi(new PlaybackCmd(evt.UniqueId, "sounds\\sv\\welcome.wav", PlaybackCmd.All));
        /// </example>
        public PlaybackCmd(string uuid, string fileName) : base(uuid)
        {
            _fileName = fileName;
        }

        public override string Command
        {
            get { return "playback"; }
        }

        public override string Arguments
        {
            get { return _fileName; }
        }
    }
}
