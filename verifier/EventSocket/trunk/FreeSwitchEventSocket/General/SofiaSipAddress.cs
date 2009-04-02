namespace FreeSwitch.EventSocket.General
{
    /// <summary>
    /// Destination as a FreeSWITCH Sofia sip address.
    /// </summary>
    public class SofiaSipAddress : Address
    {
        private readonly string _profileName;

        /// <summary>
        /// Initializes a new instance of the <see cref="SofiaSipAddress"/> class.
        /// </summary>
        /// <param name="profileName">Profile (context) name.</param>
        /// <param name="userName">User name (and if needed, IP address / domain name).</param>
        public SofiaSipAddress(string profileName, string userName)
        {
            Extension = userName;
            _profileName = profileName;
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="SofiaSipAddress"/> class.
        /// </summary>
        /// <param name="address">Another address.</param>
        public SofiaSipAddress(SipAddress address)
        {
            _profileName = address.Domain;
            Extension = address.Extension;
        }

        public override string ToString()
        {
            return "sofia/" + _profileName + "/" + Extension;
        }

        public static SofiaSipAddress Parse(string fullAddress)
        {
            string[] parts = fullAddress.Split('@');
            if (parts.Length == 2)
                return new SofiaSipAddress(parts[1], parts[0]);

            parts = fullAddress.Split('/');
            return parts.Length == 3 ? new SofiaSipAddress(parts[1], parts[2]) : null;
        }
    }
}
