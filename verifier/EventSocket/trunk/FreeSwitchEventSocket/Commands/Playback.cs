namespace FreeSwitch.EventSocket.Commands
{
    /// <summary>
    /// Play a file
    /// </summary>
    public class PlaybackCmd : SendMsg
    {
        private readonly string _fileName;
        private readonly string _dtmfAbort = string.Empty;

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

        public PlaybackCmd(string uuid, string fileName, string dtmfAbort) : base(uuid)
        {
            _fileName = fileName;
            _dtmfAbort = dtmfAbort;
        }

        public override string Command
        {
            get { return "playback"; }
        }

        public override string Arguments
        {
            get { return _fileName; }
        }

        public string DtmfAbort
        {
            get { return _dtmfAbort; }
        }
    }
}
