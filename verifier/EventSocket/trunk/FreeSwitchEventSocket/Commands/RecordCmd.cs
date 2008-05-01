using System;

namespace FreeSwitch.EventSocket.Commands
{
    public class RecordCmd : Api
    {
        //<uuid> [start|stop] <path> [<limit>]
        private readonly string _uuid;
        private readonly bool _start;
        private readonly string _path;
        private readonly int _limit;

        /// <param name="uuid">channel id</param>
        /// <param name="path">FS relative path to sound file</param>
        public RecordCmd(string uuid, string path)
        {
            if (string.IsNullOrEmpty(uuid))
                throw new ArgumentException("uuid");
            if (string.IsNullOrEmpty(path))
                throw new ArgumentException("path");

            _uuid = uuid;
            _path = path;
        }

        /// <param name="uuid">channel id</param>
        /// <param name="path">FS relative path to sound file</param>
        /// <param name="limit">number of seconds that maximum can be recorded.</param>
        public RecordCmd(string uuid, string path, int limit)
        {
            if (string.IsNullOrEmpty(uuid))
                throw new ArgumentException("uuid");
            if (string.IsNullOrEmpty(path))
                throw new ArgumentException("path");

            _uuid = uuid;
            _path = path;
            _limit = limit;
        }

        /// <param name="uuid">channel id</param>
        /// <param name="path">FS relative path to sound file</param>
        /// <param name="stop">stop the recording if true</param>
        public RecordCmd(string uuid, string path, bool stop) : this(uuid, path)
        {
            _start = !stop;
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="uuid">channel id</param>
        /// <param name="path">FS relative path to sound file</param>
        /// <param name="stop">stop the recording if true</param>
        /// <param name="limit">number of seconds that maximum can be recorded.</param>
        public RecordCmd(string uuid, string path, bool stop, int limit)
            : this(uuid, path, limit)
        {
            _start = !stop;
        }

        public override string Command
        {
            get { return "uuid_record"; }
        }

        public override string Arguments
        {
            get { return _uuid + (_start ? "start" : "stop") + " " + _path + " " + _limit; }
        }
    }
}
