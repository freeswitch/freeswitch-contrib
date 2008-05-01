using System;
using System.Collections.Generic;
using System.Text;

namespace FreeSwitch.EventSocket.General
{
    public class Address
    {
        private string _extension;

        public Address()
        {
            
        }

        public Address(string extension)
        {
            _extension = extension;
        }

        public string Extension
        {
            get { return _extension; }
            set { _extension = value; }
        }

        public override string ToString()
        {
            return Extension;
        }
    }

}
