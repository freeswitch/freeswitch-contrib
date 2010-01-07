using System;
using System.Collections.Specialized;
using System.IO;
using FreeSwitch.EventSocket.Commands;

namespace FreeSwitch.EventSocket
{
    /// <summary>
    /// Delegate used to receive a parsed event.
    /// </summary>
    /// <param name="receivedEvent">the event.</param>
    public delegate void EventHandler(EventBase receivedEvent);

    /// <summary>
    /// Delegate used to receive unparsed event messages
    /// </summary>
    /// <param name="text">text received from FreeSWITCH.</param>
    public delegate void EventsWriter(string text);

    /// <summary>
    /// Takes care of the event socket and parses the incoming events.
    /// </summary>
    public class EventManager
    {
        private readonly EventSocket _socket;
        /// <summary>
        /// An event have been received from freeswitch.
        /// </summary>
        public event EventHandler EventReceived = delegate{};
        private readonly EventsWriter _writer;
        private readonly object _logLocker = new object();
        private readonly LogWriterHandler _logWriter;

    	/// <summary>
    	/// Gets or sets display all missing event types.
    	/// </summary>
    	public bool DisplayMissingEventTypes { get; set; }

        /// <summary>
        /// FreeSWITCH password, default is "ClueCon".
        /// </summary>
        public string Password
        {
            set { _socket.Password = value; }
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="EventManager"/> class.
        /// </summary>
        /// <param name="logWriter">Can be used for logging.</param>
        public EventManager(LogWriterHandler logWriter)
        {
            _logWriter = logWriter ?? NullWriter.Write;
            _socket = new EventSocket(_logWriter);
            _writer = VoidWriter;
        }

        private static void VoidWriter(string text)
        {
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="EventManager"/> class.
        /// </summary>
        /// <param name="logWriter">Used to receive loggings.</param>
        /// <param name="writer">Used to receive unparsed event messages.</param>
        public EventManager(LogWriterHandler logWriter, EventsWriter writer)
        {
            _logWriter = logWriter ?? NullWriter.Write;
            _socket = new EventSocket(_logWriter);
            _writer = writer;
        }

        /// <summary>
        /// Starts event manger and connect to freeswitch.
        /// </summary>
        /// <param name="hostname">hostname/ipaddress colon port</param>
        public void Start(string hostname)
        {
            _socket.MessageReceived += OnMessage;
            _socket.DataReceived += OnData;
            _socket.Connect(hostname);
        }

        private void OnData(string data)
        {
            lock (_logLocker)
                _writer(data);
        }

        /// <summary>
        /// Send a command to the server.
        /// </summary>
        /// <param name="cmd"></param>
        /// <exception cref="IOException">If socket is not connected</exception>
        /// <exception cref="InvalidOperationException"></exception>
        /// <exception cref="ObjectDisposedException"></exception>
        public void Send(CmdBase cmd)
        {
            _socket.Send(cmd);
        }

        /// <summary>
        /// Sends the SendMsg command to FreeSWITCH
        /// </summary>
        /// <param name="cmd">The CMD.</param>
        public void Send(SendMsg cmd)
        {
            _socket.Send(cmd);
        }

        protected void OnMessage(PlainEventMsg msg)
        {
            NameValueCollection parameters = msg.ParseBody(true);
            string eventName = parameters["event-name"];
            if (eventName == null)
                return;

            eventName = string.Compare(parameters["event-name"], "custom", true) == 0
                            ? parameters["event-subclass"]
                            : parameters["event-name"];

            //Quickhack for Sofia::Register (convert to sofia_register)
            eventName = eventName.Replace("::", "_");
            eventName = StringHelper.UpperCaseToCamelCase(eventName);

            // Try to load the event (check if we've created an event class for the registered event)
            EventBase eb = EventBase.CreateEvent(eventName);

            // Yep, we got an event class. Let it parse the event information
            if (eb != null)
            {
                eb.SetParameters(parameters);
                eb.Parse(parameters);
                EventReceived(eb);
            }
			else if (DisplayMissingEventTypes)
                _logWriter(LogPrio.Warning, "Failed to load '" + eventName + "'.");
        }

        /// <summary>
        /// Subscribes the specified events.
        /// </summary>
        /// <param name="events">The events.</param>
        public void Subscribe(Events events)
        {
            _socket.Subscribe(events);
        }

        /// <summary>
        /// Subscribes the specified events.
        /// </summary>
        /// <param name="events">The events.</param>
        public void Subscribe(params Event[] events)
        {
            _socket.Subscribe(new Events(events));
        }
    }
}