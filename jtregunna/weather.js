/*
 * Weather application
 * Copyright (c) 2009, Jeremy Tregunna, All Rights Reserved.
 * Redistributable under any terms you like!
 *
 * Uses the Yahoo! Weather forecasting engine. http://weather.yahoo.com/
 *
 * Makes use of mod_flite, should be easy to adapt to any other TTS engine.
 */
var yweather = new Namespace("http://xml.weather.yahoo.com/ns/rss/1.0");

var rssFile = "/usr/local/freeswitch/rss/weather.rss";
var currentConditions = "Unknown";
var highTemperature = lowTemperature = currentTemperature = "0";
var doc = null;

// XXX: Big and nasty, but the values Yahoo! Weather's RSS feed gives out for "text" attributes on yweather:condition
// really don't fit in well with any method of saying things. That is, "You can expect <foo> conditions" works for
// most cases, but falls down or reads weird, when things like "tornado conditions" are inserted into <foo>. If you
// don't mind that awkwardness, you can get rid of the conditions object, and just change the code in sayWeather()
// replacing occurences of @code with @text, of course, first removing the conditions[...] that surround fc.@code.
var conditions = new Object();
conditions[0]  = "a tornado";
conditions[1]  = "tropical storm conditions";
conditions[2]  = "hurricane conditions";
conditions[3]  = "severe thunderstorms";
conditions[4]  = "thunderstorms";
conditions[5]  = "mixture of rain and snow";
conditions[6]  = "mixture of rain and sleet";
conditions[7]  = "mixture of snow and sleet";
conditions[8]  = "freezing drizzle";
conditions[9]  = "some drizzle";
conditions[10] = "freezing rain";
conditions[11] = "rain showers";
conditions[12] = "light rain showers";
conditions[13] = "snow flurries";
conditions[14] = "light snow";
conditions[15] = "blowing snow";
conditions[16] = "snow";
conditions[17] = "periods of hail";
conditions[18] = "sleet";
conditions[19] = "dusty conditions";
conditions[20] = "foggy conditions";
conditions[21] = "hazy conditions";
conditions[22] = "smokey conditions";
conditions[23] = "high winds";
conditions[24] = "windy";
conditions[25] = "cold conditions";
conditions[26] = "overcast conditions";
conditions[27] = "mostly cloudy conditions"; // Night time
conditions[28] = "mostly cloudy conditions"; // Day time
conditions[29] = "partly cloudy conditions"; // Night time
conditions[30] = "partly cloudy conditions"; // Day time
conditions[31] = "clear conditions"; // Night time
conditions[32] = "sunny conditions"; // Day time
conditions[33] = "fair conditions"; // Night time
conditions[34] = "fair conditions"; // Day time
conditions[35] = "periods of mixed rain and hail";
conditions[36] = "hot conditions";
conditions[37] = "isolated thunderstorms";
conditions[38] = "scattered thunderstorms";
conditions[39] = "scattered thunderstorms";
conditions[40] = "scattered showers";
conditions[41] = "heavy snow";
conditions[42] = "scattered snow flurries";
conditions[43] = "heavy snow";
conditions[44] = "partly cloudy conditions";
conditions[45] = "thundershowers";
conditions[46] = "snow flurries";
conditions[47] = "isolated thunderstorms";
conditions[3200] = "conditions are not available at this time";

/********** Say something **********/
say = function(something) {
	session.speak("flite", "rms", something);
}

/********** Load the XML **********/
loadXML = function(url) {
	fetchURLFile(url, rssFile);
	var fd = new FileIO(rssFile, "r");
	fd.read(3072);
	// Need to strip out the <?xml ...?> tag otherwise we get a syntax error, silly E4X
	doc = new XML(fd.data().replace(/^<\?xml\s+version\s*=\s*(["'])[^\1]+\1[^?]*\?>/, ""));
}

/********** Say the weather **********/
sayWeather = function() {
	var currentCondition = doc.channel.item.yweather::condition.@text;
	var currentTemperature = doc.channel.item.yweather::condition.@temp;
	var firstLoop = true;
	var forecast = "The current conditions are: ";
	forecast += conditions[doc.channel.item.yweather::condition.@code];
	forecast += " and it is " + currentTemperature + " degree" + ((currentTemperature != 1) ? "s" : "") + " out.\n";
	for each(var fc in doc.channel.item.yweather::forecast) {
		// XXX: Figure out why day.replace("Sun", "Sunday") et. al., doesn't actually replace the date when used here
		//      but when using the "js" spidermonkey executable, it does.
		var day = fc.@day;
		if (day == "Sun")
			day = "Sunday";
		else if (day == "Mon")
			day = "Monday";
		else if (day == "Tue")
			day = "Tuesday";
		else if (day == "Wed")
			day = "Wednesday";
		else if (day == "Thu")
			day = "Thursday";
		else if (day == "Fri")
			day = "Friday";
		else if (day == "Sat")
			day = "Saturday";
		forecast += day + "'s forecast calls for " + conditions[fc.@code] + ", with a daytime high of ";
		forecast += fc.@high + " degree" + ((fc.@high != 1) ? "s" : "") + " and a low of " + fc.@low + " degree" + ((fc.@low != 1) ? "s" : "") + ".\n";
	}
	say(forecast);
}

if (session.ready()) {
	session.answer();
	loadXML("http://weather.yahooapis.com/forecastrss?p=CAXX0531&u=c");
	sayWeather();
} 
