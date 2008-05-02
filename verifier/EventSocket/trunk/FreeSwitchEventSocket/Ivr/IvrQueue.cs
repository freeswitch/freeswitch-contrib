using System;
using System.Collections.Generic;
using FreeSwitch.EventSocket.Commands;

namespace FreeSwitch.EventSocket.Ivr
{
    internal class IvrQueue
    {
        private readonly Queue<CmdBase> _items = new Queue<CmdBase>();
        private readonly EventManager _mgr;
        private CmdBase _currentCommand;

        public IvrQueue(EventManager mgr)
        {
            if (mgr == null)
                throw new ArgumentNullException("mgr");

            _mgr = mgr;
        }

        public bool CommandRunning
        {
            get { return _currentCommand != null; }
        }

        public void Add(CmdBase cmd)
        {
            lock (_items)
                _items.Enqueue(cmd);
        }

        public void Execute(EventChannelExecute evt)
        {
        }

        public void ExecuteComplete(EventChannelExecuteComplete evt)
        {
            Console.WriteLine(evt.AppName + "(" + evt.Arguments + ") is done.");

            if (_currentCommand != null && evt.AppName != _currentCommand.Command)
            {
                Console.WriteLine("Executed something that is not ours: " + _currentCommand.Command);
                return;
            }

            // run next command
            lock (_items)
            {
                _currentCommand = null;

                if (_items.Count > 0)
                {
                    // check if next cmd is waiting on dtmf, then trigger it to start.
                    if (_items.Peek().Command == "privdtmf")
                        ((PrivateWaitDtmf) _items.Peek()).Trigger();

                    SendNextCommand();
                }
            }
        }

        /// <summary>
        /// a DTMF was pressed. Validate it towards the queue
        /// </summary>
        /// <param name="digit"></param>
        /// <returns>true if it can be processed, false if not.</returns>
        public bool GotDtmf(char digit)
        {
            bool isValid = true;

            lock (_items)
            {
                // dequeue all play commands if correct digit is pressed.
                while (_items.Count > 0)
                {
                    if (_items.Peek().GetType() == typeof (SetVariable))
                    {
                        Console.WriteLine("Is SetVariable");
                        SetVariable var = (SetVariable) _items.Peek();
                        string digits = string.Empty;
                        digits += digit;
                        if (var.Name == "playback_terminators" )
                        {
                            if (var.Value.Contains(digits))
                            {
                                _items.Dequeue();
                                if (_items.Count > 0 && _items.Peek().GetType() == typeof(PlaybackCmd))
                                    _items.Dequeue();
                            }
                            else
                                isValid = false; //digit was not one of the terminators. skip it.
                        }
                    }
                    else if (_items.Peek().GetType() == typeof(PlaybackCmd))
                    {
                        Console.WriteLine("Is PlaybackCmd");
                        PlaybackCmd cmd = (PlaybackCmd)_items.Peek();
                        string digits = string.Empty;
                        digits += digit;
                        if (cmd.DtmfAbort.Contains(digits))
                            _items.Dequeue();
                        else
                        {
                            isValid = false; //digit was not one of the terminators, skip it.
                            break;
                        }
                    }
                    else
                        break;
                }

                // and dequeue the dtmf command.
                if (_items.Count > 0 && _items.Peek().GetType() == typeof(PrivateWaitDtmf))
                    ((PrivateWaitDtmf) _items.Dequeue()).Dispose();
            }

            return isValid;
        }

        private static void OnCommandReply(CmdBase command, CommandReply reply)
        {
            command.ReplyReceived -= OnCommandReply;
            if (!reply.Success)
            {
                Console.WriteLine("Command failed: " + reply.ErrorMessage);
            }
        }

        public CmdBase Peek()
        {
            lock (_items)
            {
                return _items.Peek();
            }
        }

        public CmdBase Pop()
        {
            lock (_items)
            {
                return _items.Dequeue();
            }
        }

        private void SendNextCommand()
        {
            lock (_items)
            {
                if (_currentCommand != null)
                    throw new InvalidOperationException("A command is already running.");

                _currentCommand = _items.Dequeue();
            }

            _currentCommand.ReplyReceived += OnCommandReply;
            _mgr.Send(_currentCommand);
        }
    }
}