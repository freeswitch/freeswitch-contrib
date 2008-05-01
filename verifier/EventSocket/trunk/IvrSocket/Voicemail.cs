using System;
using System.Threading;
using FreeSwitch.EventSocket;
using FreeSwitch.EventSocket.Ivr;

namespace IvrSocket
{
    /// <summary>
    /// Very simple voicemail app =)
    /// </summary>
    class Voicemail
    {
        private readonly IvrInterface _ivr;

        public Voicemail(EventManager mgr, string uuid)
        {
            _ivr = new IvrInterface(mgr, uuid);
            _ivr.BeginInvoke(OnIvr, null, null);
        }

        private void OnIvr(IvrInterface app)
        {
            _ivr.Play("sv\\system\\record_after_beep.wav", IvrInterface.AllDigits);
            //_ivr.Sleep(5000);
            //_ivr.Record("voicemails\\" + Guid.NewGuid() + ".wav", 60);
        }
    }
}
