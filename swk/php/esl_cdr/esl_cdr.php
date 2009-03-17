<?
require_once('ESL.php');
require_once("MDB2.php");

$esl_host = '127.0.0.1';
$esl_port = '8021';
$esl_password = 'ClueCon';


$db_url ="pgsql://root@localhost/esl_cdr";

$con = MDB2::factory($db_url);
if(PEAR::isError($con)) {
    die("Error while connecting : " . $con->getMessage());
}

/* set up the ESL Connection */
$esl = new eslConnection('204.8.45.218', '8021', 'disasterisk-sucks');
$e = $esl->sendRecv("api status");
print $e->getBody();
$esl->events("plain", "channel_hangup");
for(;;) {
	print "foo - 1\n";
	$e = $esl->recvEvent();
	if ($e) {
		$lines = explode("\n", $e->serialize());
		foreach ($lines as $line){
			$temp_data = explode(": ", $line);
			
			if ($temp_data[0]){
				if ($temp_data[0] == 'variable_easy_acctcode'){
					$data['acctcode'] = $temp_data[1];
				} elseif ($temp_data[0] == 'variable_acctcode'){
					$data['acctcode'] = $temp_data[1];
				} elseif ($temp_data[0] == 'variable_accountcode'){
					$data['acctcode'] = $temp_data[1];
				} elseif ($temp_data[0] == 'variable_account_code'){
					$data['acctcode'] = $temp_data[1];
				} else {
					$data[$temp_data[0]] = $temp_data[1];
				}
			}
		}

        $insert = sprintf("INSERT into cdr (uuid, acctcode, ani, e164_ani, dnis, translated_dnis, e164_dnis, src_host, read_codec, write_codec, clid_name, ".
	"clid_number, src_sdp, dest_sdp, originate_disposition, bridge_to_uuid, endpoint_disposition, sip_hangup_disposition, term_cause, ".
	"hangup_cause, start_stamp, answer_stamp, progress_media_stamp, end_stamp, duration, billsec, progress_mediasec, billmin, ".
	"carrier_id, cust_cost, carrier_cost)".
	"VALUES".
	"('%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', %s, '%s', '%s',".
	"%s, %s, %s, %s, %s, %s, %s)",
	$data['Unique-ID'],
	$data['acctcode'],
	$data['Caller-Caller-ID-Number'],
	$data['Caller-Caller-ID-Number'],
	urldecode($data['Caller-Destination-Number']),
	urldecode($data['Caller-Destination-Number']),
	urldecode($data['Caller-Destination-Number']),
	$data['Caller-Network-Addr'],
	$data['Channel-Read-Codec-Name']."-". $data['Channel-Read-Codec-Rate'],
	$data['Channel-Write-Codec-Name']."-". $data['Channel-Write-Codec-Rate'],
	urlencode($data['Caller-Caller-ID-Name']),
	$data['Caller-Caller-ID-Number'],
	$data['variable_switch_r_sdp'],
	$data['variable_switch_m_sdp'],
	$data['variable_originate_disposition'],
	$data['variable_bridge_to'],
	$data['variable_endpoint_disposition'],
	$data['variable_sip_hangup_disposition'],
	$data['variable_sip_term_cause'],
	$data['variable_hangup_cause'],
	urldecode($data['variable_start_stamp']) . '-05',
	isset($data['variable_answer_stamp'])?'\''. urldecode($data['variable_answer_stamp']) . '-05\'':'null',
	urldecode($data['variable_progress_media_stamp']) . '-05',
	urldecode($data['variable_end_stamp']) . '-05',
	$data['variable_duration'],
	$data['variable_billsec'],
	$data['variable_progress_mediasec'],
	$data['variable_billsec'],
	"0",
	"0",
	"0"
	);

		$result = $con->query($insert);
                if(PEAR::isError($result)) {
                        echo "$insert\n";
			die();
                }
	}
}
?> 
