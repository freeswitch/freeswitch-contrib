// ActionScript file


package com.FreeSWITCH.socket {
	import com.FreeSWITCH.XML.ESL_event;
	
	import flash.errors.*;
	import flash.events.*;
	import flash.net.Socket;
	import flash.utils.ByteArray;
	import flash.utils.Dictionary;

		
	public class ESL extends Socket
	{

		private var last_data:String;
		
		private var event_callback:Dictionary = new Dictionary();
		private var api_callback:Function;
		private var command_callback:Function;
		private var esl_password:String;
		
		public function ESL(password:String = "ClueCon"):void {
			trace("init");
			esl_password = password;
			addEventListener(ProgressEvent.SOCKET_DATA, dataHandler);
		}
		
		public function addEventCallback(key:String, func:Function):void{
			event_callback[key] = func;
		}
		
		public function dataHandler(event:ProgressEvent):void {
			
			var str:String = readUTFBytes(event.bytesLoaded);
			var header:ESL_header;
			var body:String;
			
			var head_end:int;

			if (last_data != null) {
				str = last_data + str;
				last_data = null;
			}
			
			while (str != null && str.length > 0) {
			 
			 	head_end = str.indexOf("\n\n");
			 	if (head_end < 0) {
			 		last_data = str;
			 		break;
			 	}

				header = new ESL_header(str.substring(0, head_end));
				body = str.substr(head_end + 2, header.content_length());
				
				if (body.length < header.content_length() ) {
					last_data = str;
					trace("last_date: " + last_data);
					break;
				}
				
				str = str.substr(head_end + 2 + header.content_length() );
//trace(str);				
//				trace(header.content_type());
				
				if(header.content_type() == "auth/request") {
					auth(esl_password);
				}else if(header.content_type() == "api/response") {
					
					if (api_callback != null) {
						api_callback(body);
						api_callback = null;
					}
				}else if(header.content_type() == "command/reply") {
					if (command_callback != null) {
						command_callback(header.getHeader("Reply-Text"));
						command_callback = null;
					}	
				}else if(header.content_type().indexOf("text/event-") >= 0) {
					
					var event_type:String = header.content_type() == "text/event-xml" ? "xml" : "plain";
						
					var eslev:ESL_event = new ESL_event(event_type, body);
							
					for (var key:Object in event_callback)
					{			
	   					trace(key + ", " + event_callback[key]);
	   					
	   					var event_name:String, event_subclass:String;
	   					var a:Array = key.toString().split('|');
	   					event_name = a[0];
	   					event_subclass = a[1];
	   				
	   					trace("event_name: " + eslev.event_name());
	   					trace("event_subclass: " + eslev.event_subclass());
	   					
						if ( eslev.event_name() == event_name && 
							(event_subclass == '-' || eslev.event_subclass() == event_subclass)) {
							event_callback[key](eslev);
						}
					
					}
											
				}
			}
									
		}

				
		public function esl_send(str:String, callback:Function=null):void {
			var ba:ByteArray = new ByteArray();  
			ba.writeMultiByte(str + "\n\n", "UTF-8");
			
			command_callback = callback;
			writeBytes(ba);
			flush();
		}

		public function auth(pass:String, callback:Function=null):void {
			esl_send("Auth " + pass, callback);
		}
		
		public function esl_event(events:String, type:String="plain"):void {
			esl_send("event " + type + " " + events);
		}
		
		public function esl_filter(header:String, value:String, del:String=""):void {
			esl_send("filter " + del + " header " + "value");
		}
				
		public function api(str:String, callback:Function=null):void {
			api_callback = callback;
			esl_send("api " + str);
		}
		
		public function bgapi(str:String, callback:Function=null):void{
			api_callback = callback;
    		esl_send("bgapi " + str);	
		}
			
		public function execute(app:String, app_arg:String, uuid:String):void{
			var str:String = "sendmsg " + uuid + "\n\n";
			str += "call-command: execute";
			str += "execute-app-name: " + app;
			str += "execute-app-arg: " + app_arg;
			
			esl_send(str);
		}
			
	}
}
