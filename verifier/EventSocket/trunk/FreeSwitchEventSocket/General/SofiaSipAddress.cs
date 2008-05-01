namespace FreeSwitch.EventSocket.General
{
    public class SofiaSipAddress : Address
    {
        private readonly string _domain;

        public SofiaSipAddress(string domain, string userName)
        {
            Extension = userName;
            _domain = domain;
        }

        public SofiaSipAddress(SipAddress address)
        {
            _domain = address.Domain;
            Extension = address.Extension;
        }

        public override string ToString()
        {
            return "sofia/mydomain.com/" + Extension + "%" + _domain;
        }

        public static SofiaSipAddress Parse(string fullAddress)
        {
            string[] parts = fullAddress.Split('@');
            if (parts.Length == 2)
            {
                return new SofiaSipAddress(parts[1], parts[0]);
            }
            else
            {
                parts = fullAddress.Split('/');
                if (parts.Length == 3)
                {
                    return new SofiaSipAddress(parts[1], parts[2]);
                }
            }

            return null;
            
        }
    }
}
