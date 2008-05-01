using System;
using System.Collections.Specialized;
using System.IO;
using FreeSwitch.EventSocket.Commands;

namespace FreeSwitch.EventSocket
{
    public delegate void EventHandler(EventBase receivedEvent);

    public delegate void EventsWriter(string text);

    public class EventManager
    {
        private readonly EventSocket _socket = new EventSocket();
        public event EventHandler EventReceived;
        private EventsWriter _writer;
        private TextWriter _myWriter;
        public string Password
        {
            set { _socket.Password = value; }
        }

        public EventManager()
        {
            _writer = MyWriter;
            _myWriter = new StreamWriter(new FileStream("C:\\temp\\WatcherRow.log", FileMode.Create, FileAccess.Write, FileShare.ReadWrite));
        }

        private void MyWriter(string text)
        {
            _myWriter.Write(text);
        }


        public EventManager(EventsWriter writer)
        {
            _writer = writer;
        }

        public void Start(string hostname)
        {
            _socket.MessageReceived += OnMessage;
            _socket.Connect(hostname);
            _socket.DataReceived += OnData;
        }

        private void OnData(string data)
        {
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

        public void Send(SendMsg cmd)
        {
            _socket.Send(cmd);
        }

        protected void OnMessage(PlainEventMsg msg)
        {
            NameValueCollection parameters = msg.BodyToNameValue(true);
            string eventName = parameters["event-name"];
            if (eventName == null)
                return;

            if (string.Compare(parameters["event-name"], "custom", true) == 0)
                eventName = parameters["event-subclass"];
            else
                eventName = parameters["event-name"];

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
                if (EventReceived != null)
                    EventReceived(eb);

                /*
                EventChannelState channel = eb as EventChannelState;
                if (channel != null)
                {
                    Console.WriteLine("[" + eb.Name + "/" + channel.ChannelInfo.State + "] (uid: " +
                                      channel.UniqueId + ")");
                    Console.WriteLine("  ChannelInfo:");
                    Console.WriteLine("    Name: " + channel.ChannelInfo.Name);
                    Console.WriteLine("    State: " + channel.ChannelInfo.State);
                    if (channel.Caller != null)
                    {
                        Console.WriteLine("  CallerInfo:");
                        Console.WriteLine("    Id: " + channel.Caller.UniqueId);
                        Console.WriteLine("    UserName: " + channel.Caller.UserName);
                        Console.WriteLine("    CallerId: " + channel.Caller.CallerIdName + "/" +
                                          channel.Caller.CallerIdNumber);
                        Console.WriteLine("    ChannelName: " + channel.Caller.ChannelName);
                        Console.WriteLine("    DestinationNumber: " + channel.Caller.DestinationNumber);
                    }

                    if (channel.Originator != null)
                    {
                        Console.WriteLine("  Originator:");
                        Console.WriteLine("    Id: " + channel.Originator.UniqueId);
                        Console.WriteLine("    UserName: " + channel.Originator.UserName);
                        Console.WriteLine("    CallerId: " + channel.Originator.CallerIdName + "/" +
                                          channel.Originator.CallerIdNumber);
                        Console.WriteLine("    ChannelName: " + channel.Originator.ChannelName);
                        Console.WriteLine("    DestinationNumber: " + channel.Originator.DestinationNumber);
                    }
                }
                //}*/
            }
            else
                Console.WriteLine("* Failed to load '" + eventName + "'.");
        }

        public void Subscribe(Events events)
        {
            _socket.Subscribe(events);
        }

        public void Subscribe(params Event[] events)
        {
            _socket.Subscribe(new Events(events));
        }
    }
}