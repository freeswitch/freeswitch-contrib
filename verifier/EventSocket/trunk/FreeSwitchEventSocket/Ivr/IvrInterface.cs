using System;
using System.Threading;
using FreeSwitch.EventSocket.Commands;

namespace FreeSwitch.EventSocket.Ivr
{
    public class IvrInterface
    {
        public const string AllDigits = "*#0123456789";
        private readonly string _uuid;
        private string _baseSoundPath = "sounds\\";
        private string _dtmf = string.Empty;
        private EventManager _mgr;
        public event DtmfHandler DtmfReceived;
        private int _dtmfWaitCount = 0;
        private bool _abortSpeechOnDtmf;
        private bool _answered;
        private IvrInterfaceHandler _handler = null;
        private string _lastAbortDigits = "*";
        private int _defaultTimeout = 10000;
        private readonly ManualResetEvent _dtmfTimeout = new ManualResetEvent(false);
        private IvrQueue _queue;


        public IvrInterface(EventManager mgr, string uuid)
        {
            if (mgr == null)
                throw new ArgumentNullException("mgr");
            if (string.IsNullOrEmpty(uuid))
                throw new ArgumentNullException("uuid");

            _queue = new IvrQueue(mgr);
            _mgr = mgr;
            mgr.EventReceived += OnEvent;
            _uuid = uuid;

            mgr.Send(new ParkCmd(uuid));
        }

        public IAsyncResult BeginInvoke(IvrInterfaceHandler handler, AsyncCallback callback, object state)
        {
            _handler = handler;
            return handler.BeginInvoke(this, callback, state);
        }

        public void EndInvoke(IAsyncResult result)
        {
            if (_handler != null)
                _handler.EndInvoke(result);
        }

        /// <summary>
        /// dont play phrases as long as we have DTMF in the buffer.
        /// </summary>
        public bool AbortSpeechOnDtmf
        {
            get { return _abortSpeechOnDtmf; }
            set { _abortSpeechOnDtmf = value; }
        }

        /// <summary>
        /// True if IVR call have been answered.
        /// </summary>
        public bool Answered
        {
            get { return _answered; }
        }

        /// <summary>
        /// Number of seconds to wait on DTMF
        /// </summary>
        public int DefaultTimeout
        {
            get { return _defaultTimeout; }
            set { _defaultTimeout = value; }
        }

        /// <summary>
        /// Relative path under FreeSWITCH to where the sound files resides
        /// </summary>
        /// <remarks>default is "sounds\"</remarks>
        public string BaseSoundPath
        {
            get { return _baseSoundPath; }
            set { _baseSoundPath = value; }
        }

        private void OnEvent(EventBase receivedEvent)
        {
            if (!receivedEvent.IsChannelEvent)
                return;

            ChannelEvent channelEvent = (ChannelEvent) receivedEvent;
            if (channelEvent.UniqueId != _uuid)
                return;

            if (receivedEvent is EventChannelAnswer)
            {
                _answered = true;
            }
            else if (receivedEvent is EventDtmf)
            {
                EventDtmf evt = (EventDtmf) receivedEvent;
                Console.WriteLine("DTMF Event: " + evt.Digit);
                if (_queue.GotDtmf(evt.Digit))
                {
                    lock (_dtmf)
                        _dtmf += evt.Digit;

                    if (_dtmfWaitCount != 0 && _dtmf.Length >= _dtmfWaitCount)
                    {
                        Console.WriteLine("Got all DTMF: " + _dtmfWaitCount + "/" + _dtmf.Length);
                        _dtmfTimeout.Set();
                    }

                    if (DtmfReceived != null)
                        DtmfReceived(this, new DtmfEventArgs(_dtmf));
                }
            }
            else if (receivedEvent is EventChannelDestroy || receivedEvent is EventChannelHangup)
            {
                if (_mgr != null)
                {
                    _mgr.EventReceived -= OnEvent;
                    _mgr = null;
                }
            }
            else if (receivedEvent is EventChannelExecute)
            {
                EventChannelExecute exec = (EventChannelExecute)receivedEvent;
                _queue.Execute(exec);
            }
            else if (receivedEvent is EventChannelExecuteComplete)
            {
                EventChannelExecuteComplete exec = (EventChannelExecuteComplete)receivedEvent;
                _queue.ExecuteComplete(exec);
            }
        }

        /// <summary>
        /// Wait on dtmf.
        /// </summary>
        /// <param name="count">Number of digits to get</param>
        /// <param name="timeout">number of seconds to wait on all digits.</param>
        /// <remarks>You can also used the Dtmf event if you want to use asynchrounous programming.</remarks>
        /// <seealso cref="DtmfReceived"/>
        public string GetDtmf(int count, int timeout)
        {
            if (_mgr == null)
                return string.Empty;

            lock (_dtmf)
            {
                if (_dtmfWaitCount > 0)
                    throw new InvalidOperationException("Already waiting on an event.");
                _dtmfWaitCount = 1;
                _dtmfTimeout.Reset();
            }

            if (_dtmf.Length < count)
            {
                if (_queue.CommandRunning)
                        _queue.Add(new PrivateWaitDtmf(OnDtmfTimeout, timeout));

                _dtmfWaitCount = count - _dtmf.Length;
                if (!_dtmfTimeout.WaitOne(timeout, true))
                {
                    Console.WriteLine("Didn't get any DTMF: " + _dtmf);
                    _dtmf = string.Empty;
                    _dtmfWaitCount = 0;
                    return string.Empty;
                }
            }

            lock (_dtmf)
            {
                if (_dtmf.Length >= count)
                {
                    string dtmf = _dtmf.Substring(0, count);
                    if (_dtmf.Length == count)
                        _dtmf = string.Empty;
                    else
                        _dtmf = _dtmf.Remove(count);
                    _dtmfWaitCount = 0;
                    return dtmf;
                }
            }

            _dtmfWaitCount = 0;
            return string.Empty;
        }

        private void OnDtmfTimeout(PrivateWaitDtmf dtmf)
        {
            dtmf.Dispose();
            _dtmfTimeout.Set();
        }

        /// <summary>
        /// Wait on dtmf.
        /// </summary>
        /// <param name="count">Number of digits to get</param>
        /// <remarks>You can also used the Dtmf event if you want to use asynchrounous programming.</remarks>
        /// <seealso cref="DtmfReceived"/>
        /// <seealso cref="DefaultTimeout"/>
        public string GetDtmf(int count)
        {
            return GetDtmf(count, _defaultTimeout);
        }

        /// <summary>
        /// Play a sound file
        /// </summary>
        /// <param name="path">Path to sound file.</param>
        /// <seealso cref="AllDigits"/>
        /// <remarks>Prepends <see cref="BaseSoundPath"/> to path.</remarks>
        public void Play(string path)
        {
            Play(path, AllDigits);
        }

        /// <summary>
        /// Play a sound file
        /// </summary>
        /// <param name="path">Path to sound file.</param>
        /// <param name="digitsToAbortOn">DTMF digits that can abort speech.</param>
        /// <remarks>Prepends <see cref="BaseSoundPath"/> to path.</remarks>
        public void Play(string path, string digitsToAbortOn)
        {
            if (_mgr == null)
                return;

            if (_abortSpeechOnDtmf && _dtmf != string.Empty)
                return;

            if (digitsToAbortOn != _lastAbortDigits)
            {
                _mgr.Send(new SetVariable(_uuid, "playback_terminators", digitsToAbortOn));
                _lastAbortDigits = digitsToAbortOn;
            }

            _queue.Add(new PlaybackCmd(_uuid, GetSoundPath(path, false), digitsToAbortOn));
        }



        /// <summary>
        /// Record a sound file
        /// </summary>
        /// <param name="path">Where to store the sound</param>
        /// <param name="limit">Maximum number of seconds that can be recorded.</param>
        /// <param name="stop">stop an ongoing recording</param>
        public void Record(string path, int limit, bool stop)
        {
            if (_mgr == null)
                return;

            _queue.Add(new RecordCmd(_uuid, GetSoundPath(path, true), stop, limit));
        }

        /// <summary>
        /// Record a sound file
        /// </summary>
        /// <param name="path">Where to store the sound</param>
        /// <param name="limit">Maximum number of seconds that can be recorded.</param>
        public void Record(string path, int limit)
        {
            Record(path, limit, false);
        }

        /// <summary>
        /// This method can be used to build paths for different languages and such.
        /// </summary>
        /// <param name="fileName">soundfile</param>
        /// <param name="isRecording">path is to a file being recorded</param>
        /// <returns>Path to sound file</returns>
        protected virtual string GetSoundPath(string fileName, bool isRecording)
        {
            return BaseSoundPath + fileName;
        }

        /// <summary>
        /// Tell script to sleep for a while.
        /// </summary>
        /// <param name="ms">number of milliseconds</param>
        public void Sleep(int ms)
        {
            if (_mgr == null)
                return;

            _queue.Add(new SleepCmd(_uuid, ms));
        }


    }


    public delegate void IvrInterfaceHandler(IvrInterface app);
}