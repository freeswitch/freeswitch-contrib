/*
 *
 * Author Raymond Chandler (intralanman)
 * This CNAM script will work for IP CNAM queries from metrostat.net
 * put this .js file in your FS_ROOT/scripts directory and add a line to your
 * dialplan like the following:
 * <action application="javascript" data="cnam.js"/>
 *
 */

function logger(logstuff, loglevel) {
    console_log(loglevel, logstuff + "\n");
}

var cid_num_array = /\+?1?([2-9]\d{2}[0-9]\d{6})/.exec(session.caller_id_number);
if(cid_num_array && cid_num_array[1].length > 1) {
    var cid_num = cid_num_array[1];
} else {
    exit();
}
logger(cid_num, "info");

var base_url = "http://cnam1.edicentral.net/getcnam?q=C&f=S&dn=";
var complete_url = base_url + cid_num
logger(complete_url, "info");

var data = fetchURL(complete_url);
logger("CNAM is: " + data, "info");
if(data && data.length > 1) {
    session.setVariable("effective_caller_id_name", data);
}
