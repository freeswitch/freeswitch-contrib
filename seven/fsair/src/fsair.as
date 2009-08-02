		
		import com.FreeSWITCH.XML.ESL_event;
		import com.FreeSWITCH.socket.ESL;
		
		import flash.events.*;
		import flash.utils.Dictionary;
		import flash.utils.Timer;  
		
		var uuids:Dictionary = new Dictionary();
		
		var esl:ESL;
		
		var ring_win:MessageWindow = new MessageWindow();
		
		var connect_timer:Timer  = new Timer(2000);
		var re_connect_timer:Timer  = new Timer(2000);
		
		function initApp():void {
			trace(NativeApplication.nativeApplication.runtimeVersion);
			ringBox.visible = false;
			getSettings();
			esl = new ESL(esl_pass.text);
			
			connect_timer.addEventListener(TimerEvent.TIMER, post_connect);
			re_connect_timer.addEventListener(TimerEvent.TIMER, re_connect);
					
			connect();
			
		}
		
		function connect():void {  
			try  
			{  
				if (esl.connected ) {
					msg("Already Connected");
					return;
				}  

				esl.connect(esl_addr.text, (int)(esl_port.text));
				esl.addEventListener(Event.CONNECT, connectHandler);  
				esl.addEventListener(Event.CLOSE, closeHandler);  
				esl.addEventListener(ErrorEvent.ERROR, errorHandler);  
				esl.addEventListener(IOErrorEvent.IO_ERROR, ioErrorHandler);  
//				 esl.addListener(ProgressEvent.SOCKET_DATA, dataHandler);
				esl.addEventListener(SecurityErrorEvent.SECURITY_ERROR, securityErrorHandler);

				esl.addEventCallback("CUSTOM|portaudio::ringing", onRing);
				esl.addEventCallback("CHANNEL_HANGUP|-", onHangup);
			   
				connect_timer.start();
				msg("timer started");
			}
			  
			catch (error:Error)  
			{  
				msg(error.message + "\n");  
				esl.close();  
			}  
		 
		}
		
		function re_connect(event:TimerEvent):void {

			event.target.stop();
			
			event.target.delay += 2000;
			
			if (event.target.delay > 120000) {
				event.target.delay = 2000;
			}
			  
			msg("Connect faild. will reconnect in " + event.target.delay + " seconds" );
			connect();
			
		}
		
		function post_connect(event:TimerEvent):void {
			
			event.target.stop();
			
			if (!esl.connected) {
				event.target.stop();
				re_connect_timer.start();
				return;
			}
			
			msg("timer stoped");
			initDevList();
//			esl.esl_event("ALL");
			esl.esl_event("CHANNEL_HANGUP CUSTOM portaudio::ringing");
						
		}
			
		function setTitle(title:String):void {
			this.title = "FreeSWITCH Air - [" + title + ']';
		}
			
		function msg(message:String):void
		{
			logBox.text += message + "\n";
			logBox.verticalScrollPosition = logBox.maxVerticalScrollPosition;
		}
		
		function clearLogs():void {
			logBox.text = logBox.text.substr(-1, 40);
		}			  
		
		function connectHandler(event:Event):void {
			setTitle('Connected');
			msg("Connected: " + esl_addr.text + ":" + esl_port.text);
		}  
		function closeHandler(event:Event):void {
			//msg(event.type + ' ' + event.toString());
			setTitle('Disconnected');
		}
		function errorHandler(event:Event):void {
			msg(event.type + ' ' + event.toString());
			setTitle('Error');
		}
		function ioErrorHandler(event:IOErrorEvent):void {
			msg(event.type + ' ' + event.toString());
			setTitle('IOError');
			
			if (!esl.connected) {
				connect();
			}	
		}
		function securityErrorHandler(event:SecurityErrorEvent):void {
			msg(event.type + ' ' + event.toString());
			setTitle('SecurityError');

 			if (!esl.connected) {
				connect();
			}	
	
		}
		
		function onRing(eslev:ESL_event):void {
			
			var uuid:String = eslev.get_value("Unique-ID");
			var caller_id_name:String = eslev.get_value("Caller-Caller-ID-Name");
			var caller_id_number:String = eslev.get_value("Caller-Caller-ID-Number");			
			var call_id:String = eslev.get_value("call_id");
			
			uuids[uuid] = call_id;

			callerID.text = caller_id_name + ' <' + caller_id_number + '>';
			ringBox.visible = true;
			
			
			msg('Incoming call: ' + caller_id_name + '<' + caller_id_number + '>');
			msg(uuid);
			
			//how to make this work?
			//this.stage.nativeWindow.icon.bounce();
			
//			NativeApplication.nativeApplication.icon.bounce(NotificationType.CRITICAL)
     		stage.nativeWindow.notifyUser(NotificationType.CRITICAL);

			//show a ring window? 
//			ring_win.MessageLabel.text = "Incoming Call";
//			ring_win.open();
			
		}
		
		function onHangup(eslev:ESL_event):void {
			
			var uuid:String = eslev.get_value("Unique-ID");
			
			if (uuids[uuid] != null) {
				msg("hangup");
				callerID.text = "Hangup";
				delete uuids[uuid];
			}
			
		}
	  
		function sendDtmf(dtmf:String):void {
			esl.api("pa dtmf " + dtmf);
		}
		
		function switchPA():void {
			esl.api("pa switch");
		}
				
		function sendCommand():void  
		{  
		
			msg('>>> ' + command.text);
			
			if (command.text.substr(0,1) == '/') {
				esl.esl_send(command.text.substr(1), commandCallback);
			} else {
				esl.api(command.text, commandCallback)
			}
			  
		} 
		
		function commandCallback(data:String):void {
			msg(data);
		}
	
		function answer():void{
			esl.api("pa answer");
			ringBox.visible = false;
		}
			
		function hangup():void{
			esl.api("pa hangup");
			ringBox.visible = false;
		}
		
		function call():void{
			esl.api("pa call " + number.text, updateUUID);
			msg("Calling " + number.text);
			
			if( number.dataProvider.length < 1) {
				number.dataProvider.addItemAt(number.text, 0);
			} else if (number.dataProvider.getItemAt(0) != number.text ){
				number.dataProvider.addItemAt(number.text, 0);
			}
			
		}
		
		function updateUUID(data:String):void {
			
			var a:Array = new Array();
			a = data.split(":");
			
			trace(data);
			
			if (a.length == 3 && a[0] == "SUCCESS") {
				uuids[a[2].substr(0, a[2].length - 1) ] = a[1];
			} 
			
		}
		
		function initDevList():void {
			esl.api("pa devlist xml", devlistCallback);
		}
			
		function reloadDevList():void {
//			esl.api("pa rescan");
			esl.api("pa devlist xml", devlistCallback);
		}
		
		function devlistCallback(data:String):void {
//			msg(data);
			
			var xml:XML = new XML(data);
			
			inDev.dataProvider.removeAll();
			outDev.dataProvider.removeAll();
			ringDev.dataProvider.removeAll();
			
			for each (var d:XML in xml.devices.device){
				
				if (d.@inputs > 0) {
					inDev.dataProvider.addItem({ label: d.@name, value:d.@id } );
					
				}else if(d.@outputs > 0) {
					outDev.dataProvider.addItem({ label: d.@name, value:d.@id } );
					ringDev.dataProvider.addItem({ label: d.@name, value:d.@id } );
					
				}
				
				if (d.@id == xml.bindings.input.@device) {
					inDev.selectedIndex = inDev.dataProvider.length - 1;
				} else if (d.@id == xml.bindings.output.@device) {
					outDev.selectedIndex = outDev.dataProvider.length - 1;
					if (d.@id == xml.bindings.ring.@device) {
						ringDev.selectedIndex = ringDev.dataProvider.length - 1;
						 
					} 
				}
				
				

			}
			
		}
		
		function setPaDev():void{
			esl.api("pa indev #" + inDev.selectedItem.value);
			esl.api("pa outdev #" + outDev.selectedItem.value);
			esl.api("pa ringdev #" + ringDev.selectedItem.value);
		}
		
		function unloadPA():void {
			esl.api("unload mod_portaudio");
		}
		
		function loadPA():void {
			esl.api("load mod_portaudio");
		}
		
		function loopPA():void {
			esl.api("pa looptest");
		}
		
		function toggleAdvanced():void {
			if (this.width == 260 ) {
				this.width = 620;
				advancedButton.label = "Advanced <<<";
			} else {
				this.width = 260;
				advancedButton.label = "Advanced >>>";
			}
			
			
		}
		
		function _save_settings(key:String, val:String):void {
			var bytes:ByteArray = new ByteArray();  
 			bytes.writeUTFBytes(val);  
 			EncryptedLocalStore.setItem(key, bytes);  
		}
		function _get_settings(key:String):String {
			var storedValue:ByteArray = EncryptedLocalStore.getItem(key);  
			
			if (storedValue == null) {
				return null;
			}
			
 			return storedValue.readUTFBytes(storedValue.length); 
		}
		
		function saveSettings():void {
			   
 			_save_settings("freeswitch_install_path", freeswitch_install_path.text);
 			_save_settings("esl_addr", esl_addr.text);
 			_save_settings("esl_port", esl_port.text);
 			_save_settings("esl_pass", esl_pass.text);
 			
		}
		
		function getSettings():void {
			freeswitch_install_path.text = _get_settings("freeswitch_install_path") || "/usr/local/freeswitch";
			esl_addr.text = _get_settings("esl_addr") || "127.0.0.1";
			esl_port.text = _get_settings("esl_port") || "8021";
			esl_pass.text = _get_settings("esl_pass") || "ClueCon";
		}
		
		function runFS():void {
			
		}
		
		function shutdownFS():void {
			esl.api("fsctl shutdown");
		}
		
		