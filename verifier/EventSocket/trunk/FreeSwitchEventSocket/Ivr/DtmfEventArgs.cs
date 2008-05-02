using System;

namespace FreeSwitch.EventSocket.Ivr
{

    public class DtmfEventArgs : EventArgs
    {
        private string _dtmf;
        private string _dtmfCopy;

        public DtmfEventArgs(string dtmf)
        {
            _dtmf = dtmf;
            _dtmfCopy = (string)_dtmf.Clone();
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
}
