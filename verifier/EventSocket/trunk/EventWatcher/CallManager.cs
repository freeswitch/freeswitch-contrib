using System;
using System.Collections.Generic;
using System.Diagnostics;
using FreeSwitch.EventSocket;
using FreeSwitch.EventSocket.Commands;

namespace EventWatcher
{
    internal class CallManager
    {
        #region Delegates

        public delegate void CallStateHandler(CallManager mgr, CallStateEvent callEvent);

        #endregion

        private EventManager _eventMgr;
        private IDictionary<string, CallStateEvent> _events = new Dictionary<string, CallStateEvent>();

        private IDictionary<string, ExtensionMapper> _extensions = new Dictionary<string, ExtensionMapper>();
        private int _lastCallId = 0;

        public CallManager(EventManager eventSocket)
        {
            _eventMgr = eventSocket;
        }

        public event ExtensionHandler ExtensionAdded;
        public event CallHandler ExtensionChanged;
        public event ExtensionHandler ExtensionDisconnected;
        public event CallStateHandler OnCallState;

        private ExtensionMapper GetMapper(string address)
        {
            if (_extensions.ContainsKey(address))
                return _extensions[address];
            else
            {
                ExtensionMapper em = new ExtensionMapper();
                em.setter = new ExtensionSetters();
                em.extension = new Extension(address, em.setter);
                _extensions.Add(address, em);
                if (ExtensionAdded != null)
                    ExtensionAdded(em.extension);

                return em;
            }
        }

        private string ChannelDestination(string channelName)
        {
            //Check.Require(channelName, "Channel name must be set.");
            string[] parts = channelName.Split('/');
            //Check.Require(parts.Length == 3, "Channel must be sofia/<domain>/<user>");
            return parts[2];
        }

        public void OnSwitchEvents(EventBase theEvent)
        {
            if (theEvent is EventPresenceIn)
            {
                EventPresenceIn ep = (EventPresenceIn) theEvent;
                //Channel-State: CS_RING
                //Channel-Name: sofia/default/jonas%40192.168.1.102%3A5070
                //Unique-ID: 2f87ba27-2f71-d64d-8c64-9966ee894eac
                //Call-Direction: inbound
                //Answer-State: ringing
                //Event-Name: PRESENCE_IN
                CallState state = CallState.Unknown;
                string destination = string.Empty;
                if (ep.ChannelState.ChannelInfo.State == ChannelState.Ring)
                {
                    if (IsInbound(ep))
                    {
                        state = CallState.Alerting;
                        destination = ep.Caller.DestinationNumber;
                    }
                    else
                    {
                        state = CallState.Offering;
                        destination = ep.Caller.UserName;
                    }
                }
                else if (ep.ChannelState.ChannelInfo.State == ChannelState.Hangup)
                {
                    state = CallState.Disconnected;
                    if (IsInbound(ep))
                        destination = ep.Caller.DestinationNumber;
                    else
                        destination = ep.Caller.UserName;
                }
                if (state != CallState.Unknown)
                    TriggerCallState(ep.ChannelState.ChannelInfo.Address, ep.ChannelState.UniqueId, state, destination);
            }
            else if (theEvent is EventChannelAnswer)
            {
                EventChannelAnswer ea = (EventChannelAnswer) theEvent;
                string destination;
                if (IsInbound(ea))
                    destination = ea.Caller.DestinationNumber;
                else
                    destination = ea.Caller.UserName;

                TriggerCallState(ea.ChannelInfo.Address, ea.UniqueId, CallState.Connected, destination);
            }

            else if (theEvent is EventChannelDestroy)
            {
                EventChannelDestroy e = (EventChannelDestroy) theEvent;
                Debug.WriteLine("triggering CallState.Disconnected");
                TriggerCallState(e.ChannelInfo.Address, e.UniqueId, CallState.Disconnected,
                                 e.Originator.DestinationNumber);
            }
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="evt"></param>
        /// <returns>true if it's inbound to pbx (not to user)</returns>
        private bool IsInbound(EventBase evt)
        {
            return evt.Parameters["Call-Direction"] == "inbound";
        }


        public void OnSwitchEvents2(EventBase theEvent)
        {
            ChannelEvent evt = theEvent as ChannelEvent;
            EventChannelState channelState = theEvent as EventChannelState;
            if (channelState != null)
                Debug.WriteLine("ChannelState: " + channelState.ChannelInfo.State + " " + channelState.Caller.UserName +
                                " " + channelState.Caller.DestinationNumber);
            else if (evt != null && evt.ChannelInfo != null)
                Debug.WriteLine("ChannelEvent: " + evt.Name + " " + evt.ChannelInfo.Address);
            if (evt != null)
            {
                // CHANNEL_ANSWER, No originator = new outgoing call. 
                // ChannelName = the one who makes the call
                if (evt is EventChannelAnswer)
                {
                    EventChannelAnswer e = (EventChannelAnswer) evt;
                    if (e.Originator.CallerIdName != string.Empty)
                    {
                        Debug.WriteLine("Skipping emoty ChannelAnswer");
                        return; // We do only want empty ones.
                    }

                    if (!IsExternalNumber(evt.ChannelInfo.Address))
                    {
                        Debug.WriteLine("triggering CallState.Alerting");
                        TriggerCallState(evt.ChannelInfo.Address, e.UniqueId, CallState.Alerting,
                                         e.Caller.DestinationNumber);
                    }
                }

                    // CHANNEL_OUTGOING, Event for Alerting/Offering
                    // Originator = the one calling
                    // ChannelName = the one getting called.
                else if (evt is EventChannelOutgoing)
                {
                    EventChannelOutgoing e = (EventChannelOutgoing) evt;
                    if (!IsExternalNumber(evt.ChannelInfo.Address))
                    {
                        Debug.WriteLine("triggering CallState.Offering");
                        TriggerCallState(evt.ChannelInfo.Address, e.UniqueId, CallState.Offering, e.Originator.UserName);
                    }
                    if (!IsExternalNumber(e.Originator.UserName))
                    {
                        Debug.WriteLine("triggering CallState.Alerting");
                        TriggerCallState(e.Originator.UserName, e.Originator.UniqueId, CallState.Alerting,
                                         e.ChannelInfo.Address);
                    }
                }

                else if (evt is EventChannelBridge)
                {
                    EventChannelBridge e = (EventChannelBridge) evt;
                    if (!IsExternalNumber(evt.ChannelInfo.Address))
                    {
                        Debug.WriteLine("triggering CallState.Connected");
                        TriggerCallState(evt.ChannelInfo.Address, e.UniqueId, CallState.Connected,
                                         e.Caller.DestinationNumber);
                    }

                    // Trigger destination
                    if (!IsExternalNumber(e.Caller.DestinationNumber) && e.Originator.ChannelName != string.Empty)
                    {
                        Debug.WriteLine("triggering CallState.Connected");
                        TriggerCallState(ChannelDestination(e.Originator.ChannelName), e.UniqueId, CallState.Connected,
                                         e.ChannelInfo.Address);
                    }
                }

                else if (evt is EventChannelDestroy)
                {
                    EventChannelDestroy e = (EventChannelDestroy) evt;
                    Debug.WriteLine("triggering CallState.Disconnected");
                    TriggerCallState(evt.ChannelInfo.Address, e.UniqueId, CallState.Disconnected,
                                     e.Originator.DestinationNumber);
                }
            }
        }

        private bool IsExternalNumber(string number)
        {
            if (number == null || number == string.Empty)
                return false;
            return number[0] == '+';
        }

        private void TriggerCallState(string extensionOrUserName, string id, CallState state, string destination)
        {
            ExtensionMapper em = GetMapper(extensionOrUserName);
            Debug.WriteLine(String.Format("{0} -> {1}, {2} id: {3}", extensionOrUserName, destination, state, id));
            Call call = em.setter.callState(id, state, destination);
            if (ExtensionChanged != null)
                ExtensionChanged(em.extension, call);
        }

        private void OnBridgeCallId(CmdBase command, CommandReply reply)
        {
            GetVariableReply gvr = (GetVariableReply) reply;
            if (!gvr.Success)
                Debug.WriteLine("OnBridgeCallId " + reply.ErrorMessage);

            string otherChannelId = (string) command.ContextData;
            SetVariable sv = new SetVariable(otherChannelId, "gate_callid", gvr.Value);
            sv.ReplyReceived += OnCallIdReply;
            _eventMgr.Send(sv);
            command.ReplyReceived -= OnBridgeCallId;
        }

        private void OnCallIdReply(CmdBase command, CommandReply reply)
        {
            if (!reply.Success)
                Debug.WriteLine("OnCallIdReply " + reply.ErrorMessage);
            command.ReplyReceived -= OnCallIdReply;
        }

        #region Nested type: ExtensionMapper

        private class ExtensionMapper
        {
            public Extension extension;
            public ExtensionSetters setter;
        }

        #endregion
    }
}