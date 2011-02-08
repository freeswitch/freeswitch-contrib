using System;
using System.Windows;
using FreeSWITCH.Native;

namespace SampleClient {
	/// <summary>
	/// Interaction logic for MainWindow.xaml
	/// </summary>
	public partial class MainWindow : Window {
		public MainWindow() {
			InitializeComponent();
			del = new EventDelegate(actual_event_handler);
		}

		private const String sip_server = "127.0.0.1";
		private const String sip_port = "5060";
		private const String sip_user = "1001";
		private const String sip_password = "1234";

		private void btnCall_Click(object sender, RoutedEventArgs e){
			api_exec("pa", "call sofia/gateway/1/" + txtNumber.Text);
		}

		private void btnEnd_Click(object sender, RoutedEventArgs e) {
			api_exec("pa", "hangup");
		}
		public string api_exec(String cmd, String args) {
			if (API == null)
				API = new Api();
			return API.Execute(cmd, args);
		}

		private Api API;
		private IDisposable search_bind;
		private IDisposable event_bind;
		private void fs_core_end(){
			freeswitch.switch_core_destroy();
		}
		private void Window_Closing(object sender, System.ComponentModel.CancelEventArgs e) {
			if (!btnStartFreeswitch.IsEnabled)
				fs_core_end();
		}
		private void fs_core_init() {

			String err = "";
			const uint flags = (uint)(switch_core_flag_enum_t.SCF_USE_SQL | switch_core_flag_enum_t.SCF_USE_AUTO_NAT);
			freeswitch.switch_core_set_globals();
			/*Next 3 lines only needed if you want to bind to the initial event or xml config search loops */
				freeswitch.switch_core_init(flags, switch_bool_t.SWITCH_FALSE, ref err);		
				search_bind = FreeSWITCH.SwitchXmlSearchBinding.Bind(xml_search, switch_xml_section_enum_t.SWITCH_XML_SECTION_CONFIG);
				event_bind = FreeSWITCH.EventBinding.Bind("SampleClient", switch_event_types_t.SWITCH_EVENT_ALL, null, event_handler, true);
			/* End Optional Lines */
			freeswitch.switch_core_init_and_modload(flags, switch_bool_t.SWITCH_FALSE, ref err);
			
		}

		private EventDelegate del;
		private void event_handler(FreeSWITCH.EventBinding.EventBindingArgs args){
			Dispatcher.BeginInvoke(del, new object[]{args.EventObj });
		}
		private delegate void EventDelegate(switch_event evt); 

		private void actual_event_handler(switch_event evt){
			if (evt.event_id == switch_event_types_t.SWITCH_EVENT_MODULE_LOAD)
				return;

			if (evt.event_id == switch_event_types_t.SWITCH_EVENT_CUSTOM && evt.subclass_name == "sofia::gateway_state")
				blockConsole.Text = "Sofia Gateway State: " + freeswitch.switch_event_get_header(evt, "State") + "\r\n" + blockConsole.Text;
			else
				blockConsole.Text = "Event " + evt.event_id + "\r\n" + blockConsole.Text;
		}
		private void btnStartFreeswitch_Click(object sender, RoutedEventArgs e){
			btnStartFreeswitch.IsEnabled = false;
			fs_core_init();
			btnEnd.IsEnabled = btnCall.IsEnabled = true;	
		}
		private string xml_search(FreeSWITCH.SwitchXmlSearchBinding.XmlBindingArgs args){

			if (args.KeyName != "name" || args.Section != "configuration" || args.TagName != "configuration" || args.KeyValue != "sofia.conf")
				return null;
			String config = sofia_config;
			config = config.Replace("SIP_SERVER", sip_server + ":" + sip_port);
			config = config.Replace("SIP_USERNAME", sip_user);
			config = config.Replace("SIP_PASSWORD", sip_password);
			return config;
		}

		private const string sofia_config =
			@"<document type='freeswitch/xml'>
  <section name='configuration'>
    <configuration name='sofia.conf' description='Sofia Endpoint'>
      <profiles>
        <profile name='softphone'>
          <gateways>
            <gateway name='1'>
              <param name='realm' value='SIP_SERVER' />
              <param name='username' value='SIP_USERNAME' />
              <param name='password' value='SIP_PASSWORD' />
            </gateway>
          </gateways>
          <settings>
            <param name='context' value='public' />
            <param name='dialplan' value='xml' />
            <param name='codec-prefs' value='PCMA,GSM,PCMU' />
            <param name='inbound-codec-negotiation' value='generous' />
            <param name='sip-port' value='12344' />
          </settings>
        </profile>
      </profiles>
    </configuration>
  </section>
</document>";

	}
}
