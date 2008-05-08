using System;

namespace FreeSwitch.EventSocket
{
    public class EventChannelHangup : EventChannelState
    {
        private HangupCause _hangupCause;


        public HangupCause Cause
        {
            get { return _hangupCause; }
            set { _hangupCause = value; }
        }

        public override bool ParseCommand(string name, string value)
        {
            if (name == "hangupcause" || name == "hangup-cause")
            {
                string cause = StringHelper.UpperCaseToCamelCase(value);
                try
                {
                    _hangupCause = (HangupCause)Enum.Parse(typeof(HangupCause), cause, true);
                }
                catch(ArgumentException)
                {
                    _hangupCause = HangupCause.Unknown;
                }
            }
            else 
                return base.ParseCommand(name, value);

            return true;
        }

    }

    public enum HangupCause
    {
        Success,
        NoRouteTransitNet,
        NoRouteDestination,
        ChannelUnacceptable,
        CallAwardedDelivered,
        NormalClearing,
        UserBusy,
        NoUserResponse,
        NoAnswer,
        SubscriberAbsent,
        CallRejected,
        NumberChanged,
        RedirectionToNewDestination,
        ExchangeRoutingError,
        DestinationOutOfOrder,
        InvalidNumberFormat,
        FacilityRejected,
        ResponseToStatusEnquiry,
        NormalUnspecified,
        NormalCircuitCongestion,
        NetworkOutOfOrder,
        NormalTemporaryFailure,
        SwitchCongestion,
        AccessInfoDiscarded,
        RequestedChanUnavail,
        PreEmpted,
        FacilityNotSubscribed,
        OutgoingCallBarred,
        IncomingCallBarred,
        BearercapabilityNotauth,
        BearercapabilityNotavail,
        ServiceUnavailable,
        ChanNotImplemented,
        FacilityNotImplemented,
        ServiceNotImplemented,
        InvalidCallReference,
        IncompatibleDestination,
        InvalidMsgUnspecified,
        MandatoryIeMissing,
        MessageTypeNonexist,
        WrongMessage,
        IeNonexist,
        InvalidIeContents,
        WrongCallState,
        RecoveryOnTimerExpire,
        MandatoryIeLengthError,
        ProtocolError,
        Interworking,
        OriginatorCancel,
        Crash,
        SystemShutdown,
        LoseRace,
        ManagerRequest,
        BlindTransfer,
        AttendedTransfer,
        AllottedTimeout,
        UnAllocated,
        Unknown
    }

}
