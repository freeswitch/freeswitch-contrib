using System;
using System.Collections.Specialized;
using System.IO;
using System.Threading;

namespace FreeSwitch.EventSocket.Test
{
    class Program
    {
        static void Main(string[] args)
        {
			ParserTest pt = new ParserTest();
			pt.ParseMessages();
			pt.ParseAuth();
			pt.ParsePartials();
			pt.ParseMessages2();

            /*EventManager mgr = new EventManager();
            mgr.Subscribe(Events.GetChannelEvents());
            mgr.Start("localhost");
            Console.ReadLine();
            */
            Program p = new Program();
            p.Test();

        }

        private int _counter;
        private Parser _parser;
        Random _rand = new Random((int)DateTime.Now.Ticks);

        public void Test()
        {
            _parser = new Parser();
            string text = File.ReadAllText("C:\\mymsgs.txt");
            Thread[] threads = new Thread[5];
            for (int i = 0; i < 5; ++i)
                threads[i] = new Thread(FeedStream);

            while (StreamFeeder(ref text)) ;
            //Parse(text);
/*
            Parse(File.ReadAllText("C:\\events1.txt"));
            Parse(File.ReadAllText("C:\\events2.txt"));
            Parse(File.ReadAllText("C:\\events3.txt"));
*/
        }

        public void FeedStream()
        {
            
        }
        public bool StreamFeeder(ref string text)
        {
            int length;
            if (text.Length < 5)
                length = text.Length;
            else
                length = _rand.Next(text.Length / 2);

            string myText = text.Substring(0, length);
            Parse(myText);
            text = text.Remove(0, length);
            return text.Length > 0;
        }

        public void Parse(string text)
        {
            _parser.Append(text);
            PlainEventMsg msg = _parser.ParseOne();
            while (msg != null)
            {
                ++_counter;

                NameValueCollection pars = msg.ParseBody(true);
                Console.WriteLine(pars["event-name"]);

                msg = _parser.ParseOne();
            }

            
        }
    }
}
