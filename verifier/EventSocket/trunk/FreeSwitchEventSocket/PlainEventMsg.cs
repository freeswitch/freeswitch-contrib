using System;
using System.Collections.Specialized;

namespace FreeSwitch.EventSocket
{
    public class PlainEventMsg
    {
        private string _contentType = string.Empty;
        private int _contentLength = -1;
        private string _body = string.Empty;

        public string ContentType
        {
            get { return _contentType; }
            set { _contentType = value; }
        }

        public int ContentLength
        {
            get { return _contentLength; }
            set { _contentLength = value; }
        }

        public string Body
        {
            get { return _body; }
            set { _body = value; }
        }

        public bool ValidateHeader()
        {
            if (_contentType == string.Empty)
                return false;
            if (_contentType == "command/reply")
                return _body != string.Empty;

            return true;
        }

        public bool Validate()
        {
            if (!ValidateHeader())
                return false;

            if (_contentLength > 0)
                return _contentLength == _body.Length || _contentLength == _body.Length + 2; // include first and last line breaks that we skip in parsing

            return true;
        }


        public NameValueCollection ParseBody(bool urlDecodeValues)
        {
            NameValueCollection items = new NameValueCollection();
            StringParser parser = new StringParser(_body);
            string name = parser.Read(':', true);
            while (!parser.EOF && name != string.Empty)
            {
                string value = parser.ReadLine(true);
                if (items[name] == null)
                {
                    try
                    {
                        items.Add(name.ToLower(), Uri.UnescapeDataString(value));
                    }
                    catch (UriFormatException)
                    {
                        // add the value unformatted
                        items.Add(name.ToLower(), value);
                    }
                }

                name = parser.Read(':', true);
            }

            return items;
        }
    }
}