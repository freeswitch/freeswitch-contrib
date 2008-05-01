using System;

namespace EventWatcher
{
    public enum CallState
    {
        Unknown,

        /// <summary>
        /// The call has been created, but Connect has not been called yet. 
        /// A call can never transition into the idle state. 
        /// This is the initial state for both incoming and outgoing calls.
        /// </summary>
        Idle,
        /// <summary>
        /// Connect has been called, and the service provider is working on 
        /// making a connection. This state is valid only on outgoing calls. 
        /// This message is optional, because a service provider may have a 
        /// call transition directly to the connected state.
        /// </summary>
        InProgress,
        /// <summary>
        /// Call has been connected to the remote end and communication can take place.
        /// </summary>
        Connected,
        /// <summary>
        /// Call has been disconnected. There are several causes for disconnection. 
        /// See the table of valid call state transitions below.
        /// </summary>
        Disconnected,
        /// <summary>
        /// A new call has appeared, and is being offered to an application. 
        /// If the application has owner privileges on the call, it can either 
        /// call Answer or Disconnect while the call is in the offering state. 
        /// </summary>
        Offering,
        Alerting,
        /// <summary>
        /// The call is in the hold state.
        /// </summary>
        Held,
    }

    public enum HangupCause
    {
        Success,
        NormalClearing,
        NoRouteTransitNet,
        NoRouteDestination,
        Busy,
        Noanswer,
        Rejected,
        NumberChanged,
        Congestion,
    }
    /// <summary>
    /// Subclass for all call events.
    /// The events are sent as commands through the
    /// Command Dispatcher.
    /// </summary>
    [Serializable]
    public class CallStateEvent
    {
        private string _callId;
        private string _pbxId;
        private string _destination;
        private string _from;
        private CallState _state = CallState.Idle;
        private bool _isIncoming = true;
        private bool _isInternal;
        private HangupCause _hangupCause;

        public CallStateEvent()
        {
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="callId">Call identification string</param>
        /// <param name="pbxId">Id used internally in the pbx to identify the call.</param>
        /// <param name="from">Sip-Address to the one that made the call</param>
        /// <param name="destination">Sip-address to where the call was made.</param>
        public CallStateEvent(string callId, string pbxId, string from, string destination)
        {
            _callId = callId;
            _from = from;
            PbxId = pbxId;
            _destination = destination;
        }

        /// <summary>
        /// Call identification string.
        /// </summary>
        public string CallId
        {
            get { return _callId; }
            set { _callId = value; }
        }

        /// <summary>
        /// User that made the call.
        /// </summary>
        public string From
        {
            get { return _from; }
            set { _from = value; }
        }

        /// <summary>
        /// To whom the call was made.
        /// </summary>
        public string Destination
        {
            get { return _destination; }
            set { _destination = value; }
        }

        /// <summary>
        /// Current CallState.
        /// </summary>
        public CallState State
        {
            get { return _state; }
            set { _state = value; }
        }

        /// <summary>
        /// This is an incoming call.
        /// </summary>
        public bool Incoming
        {
            get { return _isIncoming; }
            set { _isIncoming = value; }
        }

        /// <summary>
        /// This is an internal call in the pbx.
        /// </summary>
        public bool Internal
        {
            get { return _isInternal; }
            set { _isInternal = value; }
        }

        public HangupCause HangupCause
        {
            get { return _hangupCause; }
            set { _hangupCause = value; }
        }

        internal string PbxId
        {
            get { return _pbxId; }
            set { _pbxId = value; }
        }
    }
}