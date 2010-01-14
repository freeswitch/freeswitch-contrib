using System;
using System.Collections.Generic;
using System.Text;

namespace FreeSwitch.EventSocket
{
	public class Parser
	{
		private readonly StringBuilder _text = new StringBuilder();
		private readonly Queue<string> _piecesToAppend = new Queue<string>();

		public string Text
		{
			get { return _text.ToString(); }
		}

		public void Append(string text)
		{
			lock (_piecesToAppend)
				_piecesToAppend.Enqueue(text);
		}

		internal static void ParseHeaders(PlainEventMsg msg, string header)
		{
			StringParser parser = new StringParser(header);
			string name = parser.Read(':', true);
			while (name != string.Empty)
			{
				switch (name)
				{
					case "Content-Length":
						msg.ContentLength = int.Parse(parser.ReadLine(true));
						break;
					case "Content-Type":
						msg.ContentType = parser.ReadLine(true);
						break;
					case "Reply-Text":
						msg.Body = parser.ReadLine(true);
						break;
				}

				// empty line == end of header
				if (parser.EOL)
					break;

				name = parser.Read(':', true);
			}
		}

		public PlainEventMsg ParseOne()
		{
			AppendPieces();

			int index = 0;
			PlainEventMsg msg = FindHeaders(ref index);
			if (msg == null)
				return null;

			// no body
			if (msg.ContentLength <= 0)
			{
				_text.Remove(0, index);
				return msg;
			}

			// too few bytes.
			if (_text.Length-index < msg.ContentLength)
				return null;

			// ignore empty line between header and body.
			IgnoreEmptyLines(ref index);

			int start = index;
			FindEmptyLine(ref index);

			int size = index - start + 2; // include \n\n

			// Validate size.
			//
			// Someone suggested that the sizes are correct and that 
			// the events was wrapped. Well. I've checked if the
			// specified position is a \n\n, and it's not.
			//
			// Don't force me to check source code of another implementation
			//
			if (size != msg.ContentLength)
			{
				// Check if the buffer contains header + body only,
				// then use the reported size, despite that our check failed.
				if (_text.Length == msg.ContentLength + start)
					size = msg.ContentLength;
				// buffer contains more than one message.
				else if (_text.Length > msg.ContentLength+start)
				{
					// Check if the reported size ends with a \n,
					// in that case we have a container event.
					if (_text[msg.ContentLength + start - 1] == '\n')
						size = msg.ContentLength;
				}
			}

			char[] body = new char[size];
			_text.CopyTo(start, body, 0, size);
			msg.Body = new string(body);
			_text.Remove(0, index);
			return msg;
		}

		private PlainEventMsg FindHeaders(ref int index)
		{
			IgnoreEmptyLines(ref index);

			int start = index;
			FindEmptyLine(ref index);
			if (start == index)
				return null; //got no header.

			char[] headerBuffer = new char[index - start];
			_text.CopyTo(start, headerBuffer, 0, index - start);
			
			PlainEventMsg eventMsg = new PlainEventMsg();
			ParseHeaders(eventMsg, new string(headerBuffer));
			return eventMsg;
		}


		private void IgnoreEmptyLines(ref int index)
		{
			while (index < _text.Length && _text[index] == '\n')
				++index;
		}

		private void FindEmptyLine(ref int index)
		{
			int pos = index;
			while (index < _text.Length-1)
			{
				if (_text[index] == '\n' && _text[index + 1] == '\n')
					return;
				++index;
			}

			index = pos;
		}

		private void AppendPieces()
		{
			lock (_piecesToAppend)
			{
				while (_piecesToAppend.Count > 0)
					_text.Append(_piecesToAppend.Dequeue());
			}
		}

		public void Clear()
		{
			_piecesToAppend.Clear();
			_text.Length = 0;
		}
	}
}
