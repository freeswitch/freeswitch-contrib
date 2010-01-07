using System;
using System.Threading;
using FreeSwitch.EventSocket.Commands;

namespace FreeSwitch.EventSocket.Ivr
{
    internal delegate void DtmfTimeoutHandler(PrivateWaitDtmf dtmf);

    internal class PrivateWaitDtmf : CmdBase, IDisposable
    {
        private Timer _timer;
        private readonly int _waitTime;
        private readonly DtmfTimeoutHandler _handler;
        
        /// <summary>
        /// DTMF timeouts should be counted AFTER a phrase has been said.
        /// this small command allows us to start the timer when so.
        /// </summary>
        /// <param name="handler">Method that will be signalled when the DTMF command goes on timeout.</param>
        /// <param name="time">Number of seconds to wait on input.</param>
        public PrivateWaitDtmf(DtmfTimeoutHandler handler, int time)
        {
            if (handler == null)
                throw new ArgumentNullException("handler");
            if (time == 0)
                throw new ArgumentException("time cannot be 0.", "time");
            if (time < 100)
                time *= 1000;
            _waitTime = time;
            _handler = handler;
        }

        public override string Command
        {
            get { return "privdtmf"; }
        }

        public override string Arguments
        {
            get { return _waitTime.ToString(); }
        }

        public void Trigger()
        {
            _timer = new Timer(OnTimeout, null, _waitTime, Timeout.Infinite);
        }

        private void OnTimeout(object state)
        {
            try
            {
                _timer.Dispose();
                _timer = null;
                _handler(this);
            }
            catch(Exception)
            {
            }
        }

        #region IDisposable Members

        ///<summary>
        ///Performs application-defined tasks associated with freeing, releasing, or resetting unmanaged resources.
        ///</summary>
        ///<filterpriority>2</filterpriority>
        public void Dispose()
        {
            if (_timer != null)
                _timer.Dispose();
            _timer = null;
        }

        #endregion
    }
}
