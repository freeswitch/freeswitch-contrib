﻿using System.Linq;
using System.Windows;

namespace FSClient {
	/// <summary>
	/// Interaction logic for Options.xaml
	/// </summary>
	public partial class Options : Window {

		public Options() {
			InitializeComponent();
		}
		private Broker broker = Broker.get_instance();
		private void Window_Loaded(object sender, RoutedEventArgs e) {

			load_devices(true);
		}
		private void load_devices(bool from_settings) {
			PortAudio.AudioDevice[] devices = broker.audio_devices;
			comboSpeakerInput.ItemsSource = comboHeadsetInput.ItemsSource = from d in devices where d.inputs > 0 select d;
			comboRingDevice.ItemsSource = comboSpeakerOutput.ItemsSource = comboHeadsetOutput.ItemsSource = from d in devices where d.outputs > 0 select d;
			string old_selected_item = comboHeadsetDevice.SelectedItem as string;
			comboHeadsetDevice.Items.Clear();
			comboHeadsetDevice.Items.Add("None");
			foreach (string headset in broker.AvailableHeadsets())
				comboHeadsetDevice.Items.Add(headset);
			comboHeadsetDevice.SelectedItem = old_selected_item;
			if (from_settings) {
				comboHeadsetInput.SelectedItem = broker.HeadsetInDev;
				comboHeadsetOutput.SelectedItem = broker.HeadsetOutDev;
				comboSpeakerInput.SelectedItem = broker.SpeakerInDev;
				comboSpeakerOutput.SelectedItem = broker.SpeakerOutDev;
				comboRingDevice.SelectedItem = broker.RingDev;
				chkIncomingBallons.IsChecked = broker.IncomingBallons;
				chkIncomingFront.IsChecked = broker.IncomingTopMost;
				chkClearDTMFS.IsChecked = broker.ClearDTMFS;
				chkUseNumbers.IsChecked = broker.UseNumberOnlyInput;

				comboHeadsetDevice.SelectedItem = broker.ActiveHeadset();
				if (comboHeadsetDevice.SelectedIndex == -1)
					comboHeadsetDevice.SelectedIndex = 0;
			}
		}
		private void SaveSettings() {
			PortAudio.AudioDevice indev = comboHeadsetInput.SelectedItem as PortAudio.AudioDevice;
			PortAudio.AudioDevice outdev = comboHeadsetOutput.SelectedItem as PortAudio.AudioDevice;
			broker.SetHeadsetDevs(indev == null ? "" : indev.name, outdev == null ? "" : outdev.name);

			indev = comboSpeakerInput.SelectedItem as PortAudio.AudioDevice;
			outdev = comboSpeakerOutput.SelectedItem as PortAudio.AudioDevice;
			broker.SetSpeakerDevs(indev == null ? "" : indev.name, outdev == null ? "" : outdev.name);
			outdev = comboRingDevice.SelectedItem as PortAudio.AudioDevice;
			broker.SetRingDev(outdev == null ? "" : outdev.name);
			broker.IncomingBallons = chkIncomingBallons.IsChecked == true;
			broker.IncomingTopMost = chkIncomingFront.IsChecked == true;
			broker.ClearDTMFS = chkClearDTMFS.IsChecked == true;
			broker.UseNumberOnlyInput = chkUseNumbers.IsChecked == true;
			broker.SetActiveHeadset(comboHeadsetDevice.SelectedItem as string);
			broker.SaveSettings();

		}
		private void Window_Closing(object sender, System.ComponentModel.CancelEventArgs e) {

		}

		private void btnSave_Click(object sender, RoutedEventArgs e) {
			SaveSettings();
			DialogResult = true;
			Close();
		}

		private void btnCancel_Click(object sender, RoutedEventArgs e) {
			DialogResult = false;
			Close();
		}

		private void btnReloadDevices_Click(object sender, RoutedEventArgs e) {
			broker.reload_audio_devices(false, false);
			load_devices(false);
		}

		private void btnSofiaSettings_Click(object sender, RoutedEventArgs e) {
			broker.edit_sofia();
		}
	}
}