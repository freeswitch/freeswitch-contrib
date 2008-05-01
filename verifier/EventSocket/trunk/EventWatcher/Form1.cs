using System;
using System.Diagnostics;
using System.Drawing;
using System.Reflection;
using System.Windows.Forms;
using FreeSwitch.EventSocket;
using FreeSwitch.EventSocket.Commands;
using FreeSwitch.EventSocket.General;

namespace EventWatcher
{
    public partial class Form1 : Form
    {
        private EventManager _eventMgr;
        private CallManager _callMgr;

        public Form1()
        {
            InitializeComponent();
            _eventMgr = new EventManager();
            _callMgr = new CallManager(_eventMgr);
            _callMgr.OnCallState += OnCallState;
            _callMgr.ExtensionAdded += OnExtensionAdded;
            _callMgr.ExtensionChanged += OnExtensionChanged;


        }

        private void OnExtensionAdded(Extension extension)
        {
            //throw new NotImplementedException();
        }

        private void OnExtensionChanged(Extension extension, Call call)
        {
            if (lv.InvokeRequired)
                BeginInvoke((MethodInvoker)delegate { OnExtensionChangedX(extension, call); });
            else
                OnExtensionChangedX(extension, call);
        }

        private void OnExtensionChangedX(Extension extension, Call call)
        {
            ListViewItem item = null;
            foreach (ListViewItem item2 in lv.Items)
            {
                if (String.Compare(item2.Name, extension.Address, true) == 0)
                {
                    item = item2;
                    break;
                }
            }
            if (item == null)//(!lv.Items.ContainsKey(extension.Address))
            {
                item = lv.Items.Add(extension.Address, extension.Address, 0);
                item.Tag = extension;
                item.SubItems.Add(call.Id);
                item.SubItems.Add(call.State.ToString());
                item.SubItems.Add(call.PreviousState.ToString());
                item.SubItems.Add("");
                item.SubItems.Add(extension.Destination);                
            }
            else
            {
                item.SubItems[2].Text = call.State.ToString();
                item.SubItems[3].Text = call.PreviousState.ToString();
            }
        }

        private void OnExtensionCallState(Extension e)
        {
            
        }
        private void OnCallState(CallManager mgr, CallStateEvent cs)
        {
            if (lv.InvokeRequired)
                BeginInvoke((MethodInvoker)delegate { OnCallStateX(mgr, cs); });
            else
                OnCallStateX(mgr, cs);
        }

        private void OnCallStateX(CallManager mgr, CallStateEvent cs)
        {
            ListViewItem item;
            if (!lv.Items.ContainsKey(cs.PbxId))
            {
                item = lv.Items.Add(cs.PbxId, cs.From, 0);
                item.Tag = cs;
                item.SubItems.Add(cs.CallId);
                item.SubItems.Add(cs.PbxId);
                item.SubItems.Add(cs.State.ToString());
                item.SubItems.Add(cs.From);
                item.SubItems.Add(cs.Destination);
            }
            else
                item = lv.Items[cs.PbxId];

            item.SubItems[1].Text = cs.CallId;
            item.SubItems[3].Text = cs.State.ToString();
            item.SubItems[4].Text = cs.Destination;

            if (cs.State == CallState.Disconnected)
                lv.Items.RemoveByKey(cs.PbxId);
        }

        private void Form1_Load(object sender, EventArgs e)
        {
            _eventMgr.Password = "ClueCon";
            _eventMgr.Start("localhost");
            _eventMgr.Subscribe(new Events(Event.All));
            _eventMgr.EventReceived += OnPreSwitchEvent;
        }

        private void OnPreSwitchEvent(EventBase theEvent)
        {
            _callMgr.OnSwitchEvents(theEvent);
            if (lvAll.InvokeRequired)
                BeginInvoke((MethodInvoker)delegate { OnSwitchEvent(theEvent); });
            else
                OnSwitchEvent(theEvent);
        }


        private string DisplayProperties(object o)
        {
            return DisplayProperties(o, 0);
        }

        private string DisplayProperties(object o, int spaces)
        {
            PropertyInfo[] pis = o.GetType().GetProperties();
            string info = string.Empty;
            foreach (PropertyInfo pi in pis)
            {
                if (pi.PropertyType == typeof (ChannelInfo) || pi.PropertyType == typeof (PartyInfo))
                {
                    info += pi.Name.PadLeft(spaces + pi.Name.Length) + "\r\n";
                    info += DisplayProperties(pi.GetValue(o, null), spaces + 2);
                }
                else
                    info += pi.Name.PadLeft(spaces + pi.Name.Length) + ": " + pi.GetValue(o, null) + "\r\n";
            }
            return info;
        }

        private void OnSwitchEvent(EventBase theEvent)
        {
            EventChannelState e = theEvent as EventChannelState;
            if (e != null)
            {

                EventChannelState channelEvent = theEvent as EventChannelState;
                if (channelEvent == null)
                    return;

                //OnSwitchEvents(theEvent);

                ListViewItem item;
                item = lvAll.Items.Add(channelEvent.ChannelInfo.Address);
                item.Tag = channelEvent;
                item.SubItems.Add(channelEvent.Originator.UserName);
                item.SubItems.Add(channelEvent.UniqueId);
                item.SubItems.Add(channelEvent.Caller.UniqueId);
                if (channelEvent.Name == "CHANNEL_STATE")
                    item.SubItems.Add(e.ChannelInfo.State.ToString());
                else
                    item.SubItems.Add(e.Name);

                if (e.GetType() == typeof(EventCodec) || theEvent.GetType() == typeof(EventChannelExecute)
                    || e.Originator.DestinationNumber == string.Empty)
                {
                    item.ForeColor = Color.Gray;
                }

            }
            log.Text = theEvent.Name;
        }

        private void AddEvent(ListBox box, ChannelEvent e)
        {
            box.Items.Insert(0, new Wrapper(e));
        }

        private void lvAll_SelectedIndexChanged(object sender, EventArgs e)
        {
            /*
            if (lvAll.SelectedItems.Count == 0)
                return;
            ListViewItem itm = lvAll.SelectedItems[0];
            MessageBox.Show(DisplayProperties(itm.Tag));
             * */
        }

        private void lvAll_ItemSelectionChanged(object sender, ListViewItemSelectionChangedEventArgs e)
        {
            if (lvAll.SelectedItems.Count == 0)
                return;
            ListViewItem itm = lvAll.SelectedItems[0];
            info.Text = DisplayProperties(itm.Tag);
        }

        #region Nested type: SwitchEventHandler

        private delegate void SwitchEventHandler(EventBase theEvent);

        #endregion

        #region Nested type: Wrapper

        private class Wrapper
        {
            public ChannelEvent e;

            public Wrapper(ChannelEvent e)
            {
                this.e = e;
            }

            public override string ToString()
            {
                return "[" + e.Name + "] " + e.ChannelInfo.State;
            }
        }

        #endregion

        private void btnOriginate_Click(object sender, EventArgs e)
        {
            //Originate org = new Originate();
            //org.Caller = new SofiaSipAddress("gauffin.com");
            //from_id.Text
        }

    }
}