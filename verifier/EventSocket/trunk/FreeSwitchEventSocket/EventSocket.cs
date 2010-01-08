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

		public delegate void DataHandler(string data);

		public delegate void EventSocketHandler(EventSocket client);

		#endregion

		private const int RetryTimeout = 5000;

		private readonly Queue<CmdBase> _commands = new Queue<CmdBase>();
		private readonly object _lockobj = new object();
		private readonly ManualResetEvent _parseEvent = new ManualResetEvent(false);
		private readonly EventParser _parser = new EventParser();
		private readonly Thread _parseThread;
		private readonly byte[] _readBuffer = new byte[8192];
		private bool _authed;
		private bool _autoConnect = true;
		private Events _events;
		private string _hostName;
		private bool _isRunning = true;
		private LogWriterHandler _logWriter;
		private bool _parsing;
		private string _password = "ClueCon";
		private int _port = 8021;
		private Socket _socket;
		private NetworkStream _stream;
		private Timer _timer;

		/// <summary>
		/// Initializes a new instance of the <see cref="EventSocket"/> class.
		/// </summary>
		/// <param name="logWriter">The log writer.</param>
		public EventSocket(LogWriterHandler logWriter)
		{
			LogWriter = logWriter ?? NullWriter.Write;
			CreateSocket();
			_parseThread = new Thread(OnParseMessages);
			_parseThread.IsBackground = true;
			_parseThread.Start();
		}

		/// <summary>
		/// Used to authenticate towards FreeSWITCH.
		/// </summary>
		/// <remarks>
		/// Default is "ClueCon"
		/// </remarks>
		public string Password
		{
			get { return _password; }
			set { _password = value; }
		}

		/// <summary>
		/// Connect automatically again if socket is disconnected.
		/// </summary>
		public bool AutoConnect
		{
			get { return _autoConnect; }
			set { _autoConnect = value; }
		}

		public LogWriterHandler LogWriter
		{
			get { return _logWriter; }
			set { _logWriter = value ?? NullWriter.Write; }
		}

		public event DataHandler DataReceived;

		private void CreateSocket()
		{
			_socket = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);
			_socket.NoDelay = true;
			_socket.ReceiveBufferSize = 65535;
		}

		public void Stop()
		{
			_isRunning = false;
			_parseEvent.Set();
			_autoConnect = false;
			_parseThread.Join(1000);
			_socket.Close();
		}

		private void OnParseMessages()
		{
			while (true)
			{
				_parseEvent.WaitOne();
				_parseEvent.Reset();
				if (!_isRunning)
					break;

				try
				{
					ParseMessages();
				}
				catch (Exception err)
				{
					LogWriter(LogPrio.Warning,
					          "Failed to parse event message." + Environment.NewLine + "Exception: " + err +
					          Environment.NewLine + _parser.Text);
					HandleDisconnect();
				}
			}
		}

		/// <summary>
		/// A event have been received
		/// </summary>
		public event MessageHandler MessageReceived = delegate { };

		/// <summary>
		/// Socket got disconnected (due to remote peer)
		/// </summary>
		public event EventSocketHandler Disconnected = delegate { };

		/// <summary>
		/// Socket is now connected
		/// </summary>
		public event EventSocketHandler Connected = delegate { };

		/// <summary>
		/// Connect to FreeSWITCH
		/// </summary>
		/// <param name="server">hostname/ipaddress colon port</param>
		/// <example>
		/// <code>eventSocket.Connect("localhost:1234");</code>
		/// </example>
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

		/// <summary>
		/// Connect to the previously specified host.
		/// </summary>
		public void Connect()
		{
			if (_hostName == null)
				throw new InvalidOperationException("Must call the other Connect method first to set hostname/port.");

			try
			{
				TryConnect();
				LogWriter(LogPrio.Debug, "Connected to freeswitch.");
			}
			catch (InvalidOperationException err)
			{
				LogWriter(LogPrio.Debug, "Failed to connect to freeswitch, reason: " + err.Message);
				HandleDisconnect();
			}
			catch (SocketException err)
			{
				LogWriter(LogPrio.Debug, "Failed to connect to freeswitch, reason: " + err.Message);
				HandleDisconnect();
			}
		}

		private void TryConnect()
		{
			lock (_lockobj)
			{
				_socket.Connect(_hostName, _port);
				if (_stream == null)
					_stream = new NetworkStream(_socket, false);

				Connected(this);
				BeginRead();
			}
		}

		private void BeginRead()
		{
			_stream.BeginRead(_readBuffer, 0, _readBuffer.Length, OnReadCompleted, null);
		}

		private void OnReadCompleted(IAsyncResult ar)
		{
			try
			{
				ReceiveWrapper(ar);
			}
			catch (Exception err)
			{
				_logWriter(LogPrio.Error, err.ToString());
			}
		}

		private void ReceiveWrapper(IAsyncResult ar)
		{
			string inbuffer;
			try
			{
				if (_stream == null)
					return;

				int bytesRead = _stream.EndRead(ar);
				if (bytesRead == 0)
				{
					LogWriter(LogPrio.Error, "Got disconnected (0 bytes read).");
					HandleDisconnect();
					return;
				}
				inbuffer = Encoding.ASCII.GetString(_readBuffer, 0, bytesRead);
				BeginRead();
			}
			catch (Exception err)
			{
				LogWriter(LogPrio.Debug, "IO exception during read: " + err.Message);
				// Remote end disconnected.
				HandleDisconnect();
				return;
			}

			_parser.Append(inbuffer);
			if (!string.IsNullOrEmpty(inbuffer))
				DataReceived(inbuffer);

			_parseEvent.Set();
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
				if (_timer == null)
					_timer = new Timer(OnTryConnect, null, RetryTimeout, RetryTimeout);
				return;
			}

			byte[] bytes = Encoding.ASCII.GetBytes(s);
			_stream.Write(bytes, 0, bytes.Length);
		}

		// Should ONLY be called from the parsing thread.
		private void ParseMessages()
		{
			PlainEventMsg msg = _parser.ParseOne();
			while (msg != null)
			{
				LogWriter(LogPrio.Trace, "MessageType: " + msg.ContentType);
				switch (msg.ContentType)
				{
					case "auth/request":
						{
							var cmd = new AuthCommand(_password);
							cmd.ReplyReceived += OnAuthed;
							_commands.Enqueue(cmd);
							Write(cmd + "\n\n");
						}
						break;
					case "api/response":
					case "command/reply":
						if (_commands.Count > 0)
						{
							CmdBase cmd = _commands.Dequeue();
							cmd.HandleReply(cmd.CreateReply(msg.Body.Trim()));
						}
						else
							LogWriter(LogPrio.Debug, "Got command reply or api response, but no actual command/api: " + msg.Body);
						break;
					default:
						MessageReceived(msg);
						break;
				}
				msg = _parser.ParseOne();
			}
		}

		private void OnAuthed(CmdBase command, CommandReply reply)
		{
			LogWriter(LogPrio.Trace, "We have not authenticated.");
			_authed = true;
			RequestEvents();
		}

		private void HandleDisconnect()
		{
			_parser.Clear();
			_authed = false;
			_socket.Close();
			CreateSocket();
			if (_stream != null)
			{
				_stream.Dispose();
				_stream = null;
			}

			Disconnected(this);
			if (!AutoConnect || _timer != null)
				return;

			LogWriter(LogPrio.Info, "Launching autoconnect timer.");
			_timer = new Timer(OnTryConnect, this, RetryTimeout, RetryTimeout);
		}

		private void OnTryConnect(object state)
		{
			try
			{
				TryConnect();
				LogWriter(LogPrio.Info, "Connected again.");
				if (_timer == null)
					return;

				Timer tmr = _timer;
				_timer = null;
				tmr.Change(Timeout.Infinite, Timeout.Infinite);
				tmr.Dispose();
			}
			catch (Exception err)
			{
				if (!(err is SocketException))
					LogWriter(LogPrio.Error, "Unexpected exception in try connect: " + err);
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
			{
				LogWriter(LogPrio.Debug, "Tried to send command when we are not connected: " + command);
				throw new IOException("Socket is not connected.");
			}

			_commands.Enqueue(command);

			// All commands need replies.
			Write(command + "\n\n");
		}
	}
}