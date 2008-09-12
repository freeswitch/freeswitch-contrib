using System;
using System.Drawing;
using System.Runtime.InteropServices;
using System.Text;
using System.Windows.Forms;
using FreeSwitch.EventSocket;

namespace EventWatcher
{
    public partial class ChannelWatcher : Form
    {
        private EventManager _mgr = new EventManager(null);
        private Color _currentColor = 0;
        StringBuilder sb = new StringBuilder();

        public ChannelWatcher()
        {
            InitializeComponent();
        }

        private void ChannelWatcher_Load(object sender, EventArgs e)
        {
            _mgr.Start("localhost");
            _mgr.Subscribe(Event.All);
            _mgr.EventReceived += OnPreSwitchEvent;
            ClearTb();
        //    AddText("bajs");
        }

        private void OnPreSwitchEvent(EventBase theEvent)
        {
            if (tv.InvokeRequired)
                BeginInvoke((MethodInvoker)delegate { OnSwitchEvent(theEvent); });
            else
                OnSwitchEvent(theEvent);
        }

        private void ClearTb()
        {
            /*
{\rtf1\ansi\ansicpg1252\deff0\deflang1053
    {\colortbl ;\red128\green0\blue0;\red0\green128\blue0;\red0\green0\blue255;\red0\green0\blue255;}
    {\fonttbl{\f0\fnil\fcharset0 Microsoft Sans Serif;}}
\viewkind4\uc1\pard\f0\fs17 Hello\cf1\b <\cf3\b0 (Wor"ld")\cf1\b >\cf0\b0 \par
}
             */
            sb.Length = 0;
            sb.Append(
                @"{\rtf1\ansi\ansicpg1252\deff0\deflang1053"
                + @"{\colortbl ;\red0\green0\blue0;\red128\green0\blue0;\red0\green128\blue0;\red0\green0\blue255;\red0\green0\blue255;}"
                + @"{\fonttbl{\f0\fnil\fcharset0 Microsoft Sans Serif;}}"
                + @"\viewkind4\uc1\pard\f0\fs20\par \par}");
            tb.Rtf = sb.ToString();
        }

        public enum Color
        {
            None,
            Black,
            Red, 
            Green,
            Blue
        }



        [DllImport("user32.dll")]
        public static extern bool LockWindowUpdate(IntPtr hWndLock);

        private void AddText(string text)
        {
            text = text.Replace("\\", "\\\\").Replace("{", "\\{").Replace("}", "\\}");
            string colored = string.Empty;
            bool inQ = false;
            bool bold = false;
            foreach (char ch in text)
            {
                Color color = Color.None;
                if (inQ)
                {
                    if (ch == '"')
                    {
                        inQ = false;
                        color = Color.Black;
                    }
                }
                else if (char.IsLetter(ch))
                    color = Color.Black;
                else if (Char.IsDigit(ch))
                    color = Color.Blue;
                else if (ch == '"')
                {
                    inQ = true;
                    color = Color.Green;
                }
                else
                {
                    bold = true;
                    color = Color.Red;
                }

                if (color != Color.None)
                    SetColor(ref colored, color, bold);

                if (ch == ')')
                    colored += ' ';
                colored += ch;
                if (ch == ')')
                    colored += ' ';
            }
            sb.Length = sb.Length - 5;
            sb.AppendLine(colored);
            sb.Append("\\par\\par}");
            LockWindowUpdate(tb.Handle);
            tb.Rtf = sb.ToString();
            tb.SelectionStart = tb.Text.Length;
            tb.ScrollToCaret();
            LockWindowUpdate(IntPtr.Zero);
            //ScrollToEnd(tb);
            
        }

        private void SetColor(ref string text, Color color, bool bold)
        {
            if (color != _currentColor)
            {
                text += "\\cf" + (int) color;
                //if (bold)
                //    text += "\\b";
                text += " ";
                _currentColor = color;
            }
        }

        private void OnSwitchEvent(EventBase theEvent)
        {
            AddText(theEvent.ToString());

            ChannelEvent evt = theEvent as ChannelEvent;
            if (evt == null)
                return;

            TreeNode node = tv.Nodes[evt.UniqueId];
            if (node == null)
            {
                tv.Nodes.Add(evt.UniqueId, evt.ToString());
                node = tv.Nodes[evt.UniqueId];
                node.Tag = evt;
                node.ToolTipText = evt.ToString().Replace(".", "\r\n");
            }
            node.Nodes.Add(new TreeNode(evt.ToString()));
        }

        private void toolStripLabel1_Click(object sender, EventArgs e)
        {
            tv.Nodes.Clear();
            ClearTb();
        }
    }
}