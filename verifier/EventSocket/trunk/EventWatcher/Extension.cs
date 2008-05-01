using System;
using System.Collections.Generic;

namespace EventWatcher
{
    public delegate void ExtensionHandler(Extension extension);
    public delegate void CallHandler(Extension extension, Call call);
    public delegate Call CallStateSetter(string id, CallState state, string destination);

    /// <summary>
    /// Used to set states for the extension.
    /// We are using this method since we do not want anything else
    /// but the CallMgr to be able to set stuff.
    /// </summary>
    /// <param name="forwardAll">null if it has changed, else the new forward destination</param>
    /// <param name="forwardBusy">null if it has changed, else the new forward destination</param>
    /// <param name="forwardNoAnswer">null if it has changed, else the new forward destination</param>
    public delegate void ForwardSetter(string forwardAll, string forwardBusy, string forwardNoAnswer);

    public class ExtensionSetters
    {
        public CallStateSetter callState;
        public ForwardSetter forwardSetter;
    }

    /// <summary>
    /// Class representing an extension.
    /// </summary>
    [Serializable]
    public class Extension
    {
        private readonly string _address = string.Empty;
        private readonly IDictionary<string, Call> _calls = new Dictionary<string, Call>();
        private readonly string _destination = string.Empty;
        private string _forwardAll = string.Empty;
        private string _forwardBusy = string.Empty;
        private string _forwardNoAnswer = string.Empty;
        private ExtensionSetters _handler;

        public Extension(string address)
        {
            _address = address;
        }

        public Extension(string address, ExtensionSetters setters)
        {
            _address = address;
            Handler = setters;
        }

        public bool IsBusy
        {
            get
            {
                foreach (KeyValuePair<string, Call> pair in _calls)
                {
                    if (pair.Value.State != CallState.Disconnected
                        && pair.Value.State != CallState.Held)
                        return false;
                }
                return true;
            }
        }

        /// <summary>
        /// Determines if this extension is forwarded in any way
        /// </summary>
        public bool IsForwarded
        {
            get
            {
                return _forwardNoAnswer != string.Empty
                       || _forwardBusy != string.Empty
                       || _forwardAll != string.Empty;
            }
        }

        public string ForwardAll
        {
            get { return _forwardAll; }
        }

        public string ForwardBusy
        {
            get { return _forwardBusy; }
        }

        public string ForwardNoAnswer
        {
            get { return _forwardNoAnswer; }
        }

        public string Destination
        {
            get { return _destination; }
        }

        public string Address
        {
            get { return _address; }
        }

        public ExtensionSetters Handler
        {
            get { return _handler; }
            set
            {
                if (value != null)
                    //Check.Require(_handler == null, "Handler can only be assigned one time.");

                    _handler = value;
                if (value != null)
                {
                    value.callState = SetCallState;
                    value.forwardSetter = SetForward;
                }
            }
        }

        private Call SetCallState(string id, CallState state, string destination)
        {
            bool wasBusy = IsBusy;

            Call call;
            if (!_calls.ContainsKey(id))
                call = new Call(id);
            else
                call = _calls[id];

            call.SetCallState(state);
            call.SetDestination(destination);

            if (CallChanged != null)
                CallChanged(this, call);

            if (state == CallState.Disconnected)
                _calls.Remove(id);


            if (wasBusy != IsBusy && BusyStateChanged != null)
                BusyStateChanged(this);

            return call;
        }

        private void SetForward(string forwardAll, string forwardBusy, string forwardNoAnswer)
        {
            if (forwardAll != null)
                _forwardAll = forwardAll;
            if (forwardBusy != null)
                _forwardBusy = forwardBusy;
            if (forwardNoAnswer != null)
                _forwardNoAnswer = forwardNoAnswer;
            if (ForwardChanged != null)
                ForwardChanged(this);
        }

        public event ExtensionHandler BusyStateChanged;
        public event CallHandler CallChanged;
        public event ExtensionHandler ForwardChanged;
    }
}