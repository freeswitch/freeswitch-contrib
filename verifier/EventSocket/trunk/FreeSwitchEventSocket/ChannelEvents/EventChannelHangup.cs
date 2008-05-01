using System;
using System.Collections.Generic;
using System.Text;

namespace FreeSwitch.EventSocket
{
    public class EventChannelHangup : EventChannelState
    {
        private Causes m_hangupCause;

        public enum Causes
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
            AllottedTimeout            
        }

        public Causes Cause
        {
            get { return m_hangupCause; }
            set { m_hangupCause = value; }
        }

        public override bool ParseCommand(string name, string value)
        {
            if (name == "hangupcause" || name == "hangup-cause")
            {
                string cause = StringHelper.UpperCaseToCamelCase(value);
                m_hangupCause = (Causes)Enum.Parse(typeof(Causes), cause);
            }
            else 
                return base.ParseCommand(name, value);

            return true;
        }

    }
}
