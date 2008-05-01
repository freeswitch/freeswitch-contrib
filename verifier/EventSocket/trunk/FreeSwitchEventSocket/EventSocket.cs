using System;
using System.Collections.Generic;
using System.IO;
using System.Net.Sockets;
using System.Text;
using System.Threading;
using FreeSwitch.EventSocket.Commands;

namespace FreeSwitch.EventSocket
{
    public delegate void MessageHandler(PlainEventMsg msg);

    public class EventSocket
    {
        #region Delegates

        public delegate void EventSocketHandler(EventSocket client);

        public delegate void DataHandler(string data);

        #endregion

        private readonly Queue<CmdBase> _commands = new Queue<CmdBase>();
        private bool _autoConnect = true;
        private Events _events;
        private string _hostName;
        private readonly EventParser _parser = new EventParser(string.Empty);
        public event DataHandler DataReceived;
        private string _password = "ClueCon";
        private int _port = 8021;
        private object _lockobj = new object();
        private readonly byte[] _readBuffer = new byte[8192];
        private string _temp = string.Empty;
        private Socket _socket = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);
        private NetworkStream _stream;
        private Timer _timer;
        private bool _authed = false;
        const int RetryTimeout = 5000;

        public string Password
        {
            get { return _password; }
            set { _password = value; }
        }

        public bool AutoConnect
        {
            get { return _autoConnect; }
            set { _autoConnect = value; }
        }

        public event MessageHandler MessageReceived;

        public event EventSocketHandler Disconnected;
        public event EventSocketHandler Connected;

        public void Connect(string server)
        {
            int pos = server.IndexOf(':');
            if (pos == -1)
                _hostName = server;
            else
            {
                _port = int.Parse(server.Substring(pos + 1));
                _hostName = server.Substring(0, pos);
            }

            Connect();
        }

        public void Connect()
        {
            if (_hostName == null)
                throw new InvalidOperationException("Must call the other Connect method first to set hostname/port.");

            try
            {
                TryConnect();
            }
            catch (SocketException)
            {
                Console.WriteLine("EventSocket: Connect failed");
                HandleDisconnect();
            }
        }

        private void TryConnect()
        {
            lock (_lockobj)
            {
                Console.WriteLine("EventSocket.TryConnect: Connecting to " + _hostName + ":" + _port);
                
                _socket.Connect(_hostName, _port);
                if (_stream == null)
                    _stream = new NetworkStream(_socket, false);

                if (Connected != null)
                    Connected(this);
                BeginRead();

                if (_timer != null)
                {
                    Timer tmr = _timer;
                    _timer = null;
                    tmr.Change(Timeout.Infinite, Timeout.Infinite);
                    tmr.Dispose();
                }
            }
        }

        private void BeginRead()
        {
            _stream.BeginRead(_readBuffer, 0, _readBuffer.Length, OnReadCompleted, null);
        }

        private void OnReadCompleted(IAsyncResult ar)
        {
            int bytesRead;
            try
            {
                bytesRead = _stream.EndRead(ar);
                if (bytesRead == 0)
                {
                    HandleDisconnect();
                    return;
                }
            }
            catch (IOException)
            {
                // Remote end disconnected.
                HandleDisconnect();
                return;
            }

            string text = Encoding.ASCII.GetString(_readBuffer, 0, bytesRead);
            if (DataReceived != null)
                DataReceived(text);
            _temp += text;
            _parser.Append(text);
            ParseMessages();

            _stream.BeginRead(_readBuffer, 0, _readBuffer.Length, OnReadCompleted, null);
        }

        /// <summary>
        /// Subscribe on the specified events.
        /// </summary>
        /// <param name="events">The events.</param>
        public void Subscribe(Events events)
        {
            _events = events;
            if (_socket.Connected && _authed)
                RequestEvents();
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="s"></param>
        /// <exception cref="InvalidOperationException"></exception>
        /// <exception cref="ObjectDisposedException"></exception>
        private void Write(string s)
        {
            if (_socket == null)
                return;
            if (!_socket.Connected)
            {
                try
                {
                    Console.WriteLine("EventSocket.Write: Trying to connect eventsocket.");
                    TryConnect();
                    Console.WriteLine("EventSocket.Write: Connected.");
                }
                catch (SocketException)
                {
                    return;
                }
            }

            byte[] bytes = Encoding.ASCII.GetBytes(s);
            _stream.Write(bytes, 0, bytes.Length);
        }

        private void ParseMessages()
        {
            PlainEventMsg msg = _parser.ParseMessage();
            while (msg != null)
            {
                if (msg.ContentType == "auth/request")
                {
                    AuthCommand cmd = new AuthCommand(_password);
                    cmd.OnReply += OnAuthed;
                    _commands.Enqueue(cmd);
                    Write(cmd + "\n\n");
                }
                else if (msg.ContentType == "command/reply"
                         || msg.ContentType == "api/response")
                {
                    if (_commands.Count > 0)
                    {
                        CmdBase cmd = _commands.Dequeue();
                        Console.WriteLine("Reply to " + cmd.GetType().Name + ": " + msg.Body);
                        cmd.HandleReply(cmd.CreateReply(msg.Body));
                    }
                    else
                    {
                        Console.ForegroundColor = ConsoleColor.Red;
                        Console.WriteLine("Got command reply or api response, but no actual command/api: " + msg.Body);
                        Console.ForegroundColor = ConsoleColor.Gray;
                    }
                }
                else
                {
                    if (MessageReceived != null)
                        MessageReceived(msg);
                }
                msg = _parser.ParseMessage();
            }
        }

        private void OnAuthed(CmdBase command, CommandReply reply)
        {
            _authed = true;
            RequestEvents();
        }

        private void HandleDisconnect()
        {
            _parser.Clear();
            _authed = false;
            _socket.Close();
            _socket = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);
            if (_stream != null)
            {
                _stream.Dispose();
                _stream = null;
            }

            if (Disconnected != null)
                Disconnected.Invoke(this);
            if (AutoConnect && _timer == null)
            {
                Console.WriteLine("EventSocket: Launching timer.");
                _timer = new Timer(TryConnect, this, RetryTimeout, RetryTimeout);
            }
        }

        private void RequestEvents()
        {
            if (_events.Count <= 0 && _authed)
                return;

            Write("event plain " + GetEventString() + "\n\n");
        }

        private string GetEventString()
        {
            string events = string.Empty;
            for (int i = 0; i < _events.Count; ++i)
                events += StringHelper.CamelCaseToUpperCase(_events[i].ToString()) + " ";
            return events;
        }

        /// <summary>
        /// Send a command to FreeSwitch.
        /// Do you want a response? Then use the delegate in the Command class.
        /// </summary>
        /// <param name="command">Command to send.</param>
        /// <exception cref="IOException">If socket is not connected.</exception>
        /// <exception cref="InvalidOperationException"></exception>
        /// <exception cref="ObjectDisposedException"></exception>
        public void Send(CmdBase command)
        {
            if (!_socket.Connected)
                throw new IOException("Socket is not connected.");

            _commands.Enqueue(command);

            // All commands need replies.
            Write(command + "\n\n");
        }

        private static void TryConnect(object state)
        {
            EventSocket client = (EventSocket) state;
            try
            {
                Console.WriteLine("EventSocket.Timer: Trying to connect eventsocket.");
                client.TryConnect();
                Console.WriteLine("EventSocket.Timer: Connected.");
            }
            catch (SocketException err)
            {
                Console.WriteLine("Eventsocket.TryConnect: " + err.Message);
            }
        }
    }
}