using System;
using System.Collections.Generic;
using System.Text;

namespace FreeSwitch.EventSocket.General
{
    public class SipAddress : Address
    {
        private string _domain;

        public SipAddress(string domain, string extension)
            : base(extension)
        {
            _domain = domain;
        }

        public string Domain
        {
            get { return _domain; }
            set { _domain = value; }
        }

        public override string ToString()
        {
            return Extension + "@" + Domain;
        }

        public static SipAddress Parse(string fullAddress)
        {
            string[] parts = fullAddress.Split('@');
            if (parts.Length == 2)
            {
                return new SipAddress(parts[1], parts[0]);
            }
            return null;
        }
    }
}
