using EventSocketParser;

namespace FreeSwitch.EventSocket
{
    public class EventParser
    {
        private PlainEventMsg _msg;
        private readonly StringParser _parser;

        public EventParser(string text)
        {
            _parser = new StringParser(text);
        }

        public PlainEventMsg ParseMessage()
        {
            _parser.ClearSavedPositions();
            _msg = new PlainEventMsg();

            // Save pos if we cant parse the whole message
            _parser.SavePos();

            ReadHeader();
            if (!_msg.ValidateHeader())
            {
                _parser.RestorePos();
                return null;
            }

            // skip empty line between body and header
            _parser.ReadLine(true);

            // we got no body.
            if (_msg.ContentLength == -1)
                return _msg;

            // Unfortunately, \n\n doesnt appear after all content.
            if (_msg.ContentLength != 0)
                _msg.Body = _parser.Read(_msg.ContentLength);
            else
                _msg.Body = _parser.ReadToEmptyLine();

            // assume that we have not got a complete packet yet.
            if (!_msg.Validate())
            {
                _parser.RestorePos();
                return null;
            }

            return _msg;
        }

        public bool ReadHeader()
        {
            // skip empty lines
            while (!_parser.EOF && _parser.PeekWord(true, true) == string.Empty)
                _parser.ReadLine(false);

            if (_parser.EOF)
                return false;

            string name = _parser.Read(':', true);
            while (name != string.Empty)
            {
                switch (name)
                {
                    case "Content-Length":
                        _msg.ContentLength = int.Parse(_parser.ReadLine(true));
                        break;
                    case "Content-Type":
                        _msg.ContentType = _parser.ReadLine(true);
                        break;
                    case "Reply-Text":
                        _msg.Body = _parser.ReadLine();
                        break;
                }

                // empty line == end of header
                if (_parser.EOL)
                    break;

                name = _parser.Read(':', true);
            }

            return _msg.ContentType != string.Empty;
        }

        public void Append(string value)
        {
            _parser.RemoveUsedContent();
            _parser.Append(value);
        }

        public void Clear()
        {
            _parser.Clear();
        }
    }
}