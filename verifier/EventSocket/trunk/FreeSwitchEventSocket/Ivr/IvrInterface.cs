using System;
using System.Threading;
using FreeSwitch.EventSocket.Commands;

namespace FreeSwitch.EventSocket.Ivr
{
    public class IvrInterface
    {
        public const string AllDigits = "*#0123456789";
        private readonly string _uuid;
        private readonly string _baseSoundPath = "sounds\\";
        private string _dtmf = string.Empty;
        private EventManager _mgr;
        public event DtmfHandler DtmfReceived;
        private readonly int _dtmfWaitCount = 0;
        private ManualResetEvent _dtmfWaitEvent;
        private bool _abortSpeechOnDtmf;
        private bool _answered;
        private IvrInterfaceHandler _handler = null;
        private string _lastAbortDigits = "*";

        public IvrInterface(EventManager mgr, string uuid)
        {
            if (mgr == null)
                throw new ArgumentNullException("mgr");
            if (string.IsNullOrEmpty(uuid))
                throw new ArgumentNullException("uuid");

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

        public bool Answered
        {
            get { return _answered; }
        }

        private void OnEvent(EventBase receivedEvent)
        {
            if (receivedEvent is EventChannelAnswer)
            {
                _answered = true;
            }
/*            else if (receivedEvent is EventDtmfStatus)
            {
                // Let's always dequeue dtmf.
                EventDtmfStatus evt = (EventDtmfStatus) receivedEvent;
                if (evt.UniqueId == _uuid)
                    _mgr.Send(new GetDtmfCmd(_uuid, evt.Count));
            }*/
            else if (receivedEvent is EventDtmf)
            {
                EventDtmf evt = (EventDtmf) receivedEvent;
                lock (_dtmf)
                    _dtmf += evt.Digit;
                if (_dtmfWaitCount >= _dtmf.Length)
                    _dtmfWaitEvent.Set();

                if (DtmfReceived != null)
                    DtmfReceived(this, new DtmfEventArgs(_dtmf));
            }
            else if (receivedEvent is EventChannelDestroy || receivedEvent is EventChannelHangup)
            {
                if (_mgr != null)
                {
                    _mgr.EventReceived -= OnEvent;
                    _mgr = null;
                }
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
            if (_dtmfWaitEvent != null)
                throw new InvalidOperationException("Already waiting on an event.");

            if (_dtmf.Length < count)
            {
                _dtmfWaitEvent = new ManualResetEvent(false);
                if (!_dtmfWaitEvent.WaitOne(timeout * 1000, true))
                {
                    _dtmf = string.Empty;
                    return string.Empty;
                }
            }

            lock (_dtmf)
            {
                if (_dtmf.Length >= count)
                {
                    string dtmf = _dtmf.Substring(0, count);
                    _dtmf = _dtmf.Remove(count);
                    return dtmf;
                }
            }

            return string.Empty;
        }

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

            _mgr.Send(new PlaybackCmd(_uuid, _baseSoundPath + path));
        }

        public void Record(string path, bool stop, int limit)
        {
            if (_mgr == null)
                return;
            _mgr.Send(new RecordCmd(_uuid, _baseSoundPath + path, stop, limit));
        }

        public void Record(string path, int limit)
        {
            if (_mgr == null)
                return;
            _mgr.Send(new RecordCmd(_uuid, _baseSoundPath + path, limit));
        }

        public void Sleep(int ms)
        {
            if (_mgr == null)
                return;
            _mgr.Send(new SleepCmd(_uuid, ms));
        }
    }

    public class DtmfEventArgs : EventArgs
    {
        private string _dtmf;
        private string _dtmfCopy;

        public DtmfEventArgs(string dtmf)
        {
            _dtmf = dtmf;
            _dtmfCopy = (string) _dtmf.Clone();
        }

        /// <summary>
        /// You cannot remove digits from the buffer (since it's a copy of the real buffer).
        /// </summary>
        /// <seealso cref="RemoveDigits"/>
        public string Dtmf
        {
            get { return _dtmfCopy; }
        }

        /// <summary>
        /// Remove digits from the real buffer.
        /// </summary>
        /// <param name="count">number of digits to remove.</param>
        public void RemoveDigits(int count)
        {
            lock (_dtmf)
            {
                if (count > _dtmf.Length)
                    _dtmf = string.Empty;
                else
                    _dtmf = _dtmf.Remove(0, count);
                _dtmfCopy = (string)_dtmf.Clone();
            }
        }
    }

    public delegate void DtmfHandler(object source, DtmfEventArgs args);

    public delegate void IvrInterfaceHandler(IvrInterface app);
}
