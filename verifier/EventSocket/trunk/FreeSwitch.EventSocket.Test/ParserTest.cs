using System;
using System.Collections.Generic;
using System.Text;

namespace FreeSwitch.EventSocket.Test
{
    public class ParserTest
    {
    	private const string Auth = "Content-Type: auth/request\n\n";

    	private const string Messages =
    		"Content-Type: event\nContent-Length:10\n\n1234567890\n\nContent-Type:event\nContent-Length:0\n\n";
		private const string Messages2 =
			"Content-Type:event\nContent-Length:0\n\nContent-Type: event\nContent-Length:10\n\n1234567890\n\n";

		private const string Partial1 =
			"Content-Type: event\nContent-Length:10\n\n12345";
		private const string Partial2 =
			"67890\n\nContent-Type:event\nContent-Length:0\n\n";

		public void ParseAuth()
		{
			Parser parser = new Parser();
			parser.Append(Auth);
			PlainEventMsg msg = parser.ParseOne();

		}

		public void ParseMessages()
		{
			Parser parser = new Parser();
			parser.Append(Messages);
			var msg = parser.ParseOne();

			msg = parser.ParseOne();
		}

		public void ParseMessages2()
		{
			Parser parser = new Parser();
			parser.Append(Messages2);
			var msg = parser.ParseOne();

			msg = parser.ParseOne();
		}

		public void ParsePartials()
		{
			Parser parser = new Parser();
			parser.Append(Partial1);
			var msg = parser.ParseOne();
			parser.Append(Partial2);
			msg = parser.ParseOne();
			msg = parser.ParseOne();
		}
    }
}
