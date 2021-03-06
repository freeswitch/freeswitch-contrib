using System;
using System.Collections.Generic;
using System.IO;
using System.Text;

namespace FreeSwitch.EventSocket
{
    public class EventParser
    {
        private static readonly char[] BugWorkaround = {'C', 'o', 'n', 't', 'e', 'n', 't', '-'};
        private readonly StringBuilder _text = new StringBuilder();
        private readonly Queue<string> _piecesToAppend = new Queue<string>();

        public string Text
        {
            get { return _text.ToString(); }
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
        /*
        private int IsLineBreaks(int pos)
        {
            if (pos > 0 && Text[pos] == '\n' && Text[pos - 1] == '\n')
                return 1;
            if (pos > 3 && Text[pos] == '\n' && Text[pos - 1] == '\r'
                && Text[pos - 2] == '\n' && Text[pos - 3] == '\r')
                return 3;
            return 0;
        }
        */
        public void Clear()
        {
            lock (Text)
                _text.Length = 0;
        }

        private void IgnoreLineBreaks(ref int i)
        {
            while (i < _text.Length && (_text[i] == '\n' || _text[i] == '\r'))
                ++i;
        }

        /// <summary>
        /// 
        /// </summary>
        /// <returns></returns>
        /// <exception cref="InvalidDataException">If parsing failed due to invalid format.</exception>
        public PlainEventMsg ParseOne()
        {
            // Move queue to text
            lock (_piecesToAppend)
            {
                while (_piecesToAppend.Count > 0)
                _text.Append(_piecesToAppend.Dequeue());
            }

			if (_text.Length == 0)
				return null;

            PlainEventMsg plainEvent;
            lock (_text)
            {
            	int i = 0;
				IgnoreLineBreaks(ref i);
            	_text.Remove(0, i);

            	i = FindEmptyLine(i);
				if (i == -1)
					return null;

            	// extract header
                char[] chars = new char[i];
                _text.CopyTo(0, chars, 0, i);
                string headers = new string(chars);
                IgnoreLineBreaks(ref i);

                plainEvent = new PlainEventMsg();
                ParseHeaders(plainEvent, headers);

                // we got a body?
                if (plainEvent.ContentLength > 0)
                {
                    // Start of Empty header bugfix
                    // FS seems to send a header with content-length without sending an actual body
                    // this will eat that kind of header.
                    if (_text.Length >= BugWorkaround.Length + i)
                    {
                        bool found = true;
                        for (int index = 0; index < BugWorkaround.Length; ++index)
                        {
                            if (_text[index + i] != BugWorkaround[index])
                            {
                                found = false;
                                break;
                            }
                        }
                        if (found)
                        {
                            _text.Remove(0, i);
                            Console.WriteLine("Removing empty content header.");
                            return null;
                        }
                    }
                    // end of bugfix.

                    // not enough data for body.
                    if (plainEvent.ContentLength + i > _text.Length)
                        return null;

                    // extract body

					// since Freeswitch report false sizes sometimes,
					// we need to go through all lines to find contents.
					//
					// If FreeSWITCH is actually doing something wrong and you can prove it, you should report it as a bug.
					// More likely you are misunderstanding the protocol and confusing the events with more encapsulated events
					// as a mistake. Dozens of other people have had that same mistake making an ESL client.
					// You should look in libs/esl and try to get the swig wrapper on the esl lib working instead.

                	int endOfBody = FindEmptyLine(i) + 2; // include the first and last \n
					if (endOfBody-i != plainEvent.ContentLength)
					{
						Console.WriteLine("Invalid content length, actual length: " + (endOfBody - i) + ", reported length: " +
						                  plainEvent.ContentLength);
						plainEvent.ContentLength = endOfBody - i;
					}
					chars = new char[plainEvent.ContentLength];
					_text.CopyTo(i, chars, 0, plainEvent.ContentLength);
                    plainEvent.Body = new string(chars);

                    // api/response is buggy for originate, no \n\n are appended at the end.
                    if (plainEvent.ContentType != "api/response")
                    {
                        // check for errors.
                        int pos = plainEvent.Body.IndexOf("\n\n");
                        if (pos < plainEvent.Body.Length - 2)
                        {
                            Console.WriteLine("Invalid event");
                            Console.WriteLine("Header");
                            Console.WriteLine(headers);
                            Console.WriteLine("Body");
                            Console.WriteLine(plainEvent.Body);
                            Console.WriteLine("=========================== EVERYTHING unparsed ==============================");
                            Console.WriteLine(_text);
                            throw new InvalidDataException("Invalid event: " + _text);
                        }
                    }

                    if (plainEvent.Body.Length < plainEvent.ContentLength)
                        throw new InvalidDataException("Body contents are too small!");

                    // Move forward to next header
                    i += plainEvent.ContentLength;
                    IgnoreLineBreaks(ref i);
                }


                // remove header( + body) from buffer
                _text.Remove(0, i);
            }

            return plainEvent;
        }

    	private int FindEmptyLine(int i)
    	{
    		for (; i < _text.Length - 1; ++i)
    		{
    			if (_text[i] == '\n' && _text[i + 1] == '\n')
    				return i;
    		}
    		return -1;
    	}


    	public void Append(string text)
        {
            lock (_piecesToAppend)
                _piecesToAppend.Enqueue(text);
        }
    }
}
