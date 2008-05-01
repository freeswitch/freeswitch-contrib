using System;
using System.Collections.Generic;
using System.Text;

namespace EventWatcher
{
    public class Call
    {
        private CallState _currentState = CallState.Disconnected;
        private CallState _previousState = CallState.Disconnected;
        private string _id;
        private string _destination;

        public Call(string id)
        {
            _id = id;
        }

        public string Id
        {
            get { return _id; }
            set { _id = value; }
        }

        public string Destination
        {
            get { return _destination; }
        }

        public CallState State
        {
            get { return _currentState; }
        }

        public CallState PreviousState
        {
            get { return _previousState; }
        }

        internal void SetCallState(CallState state)
        {
            _previousState = _currentState;
            _currentState = state;
        }

        internal void SetDestination(string dest)
        {
            _destination = dest;
        }

    }
}
