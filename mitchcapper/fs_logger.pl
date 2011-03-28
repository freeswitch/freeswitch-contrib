#!/usr/bin/perl
##### NOTE: You need: http://jira.freeswitch.org/browse/FS-3188
use strict;
$| = 1;
my $THREADS_SUPPORTED; #required for polling support on STDIN in windows...
my $IS_WINDOWS;
BEGIN {
	$IS_WINDOWS = $^O =~ /mswin/i;
	if ($IS_WINDOWS){
		$THREADS_SUPPORTED = eval 'use threads; 1';
		if ($THREADS_SUPPORTED){
			require threads;
			import threads;
		}
	}
}
require IO::Socket::INET;
use IO::Socket;
use Time::HiRes qw( sleep );
use POSIX ":sys_wait_h";
my ($pid,$output_buffer,$in_cleanup,$proc_stdin);
my @AUTOS = qw/-pb -do -oa -st internal -l 7/;
push @AUTOS, "-ia" if (! $IS_WINDOWS || $THREADS_SUPPORTED);

my ($DISPLAY_OUTPUT,$ACCEPT_INPUT,$PASTEBIN_USER,$FILE,$OB_AUTO,$OB_FILE,@SIP_TRACE_ON,$SOFIA_LOG_LEVEL,$CLEANUP_COMMANDS,$DEBUG_MODE);

$SIG{INT} = \&cleanup;
my $FS_CLI = $IS_WINDOWS ? "fs_cli.exe" : "./fs_cli";
$FS_CLI =  $IS_WINDOWS ? "c:/program files/Freeswitch/fs_cli.exe" : "/usr/local/freeswitch/bin/fs_cli" if (! -e $FS_CLI);
die "Unable to find fs_cli in current directory or in default location of: $FS_CLI" if (! -e $FS_CLI);
my @CMD = qw/-b/;
my (@EXEC_ON_CONNECT,@EXEC_ON_QUIT);


sub usage(){
	my $auto_str = join " ", @AUTOS;
	my $usage = qq~
    Usage: fs_logger.pl options
      -A, --auto                     Auto mode, equiv of $auto_str
      -h, --help                     Usage Information
      -H, --host=hostname            Host to connect
      -P, --port=port                Port to connect (1 - 65535)
      -u, --user=user\@domain         user\@domain
      -p, --password=password        Password
      -x, --execute=command          Execute Command on connect (can be used multiple times)
      -X, --quit-execute=command     Execute Command when quitting (can be used multiple times)	 
      -l, --loglevel=command         Log Level
      -d, --debug=level              fs_cli Debug Level (0 - 7)
      -q, --quiet                    Disable logging
      -r, --retry                    Retry connection on failure
      -R, --reconnect                Reconnect if disconnected
      -f, --file=<file>              Output file
      -pb --paste-bin[=<name>]       Post to FreeSWITCH Paste Bin (optional name to post as)
      -st --sip-trace[=<profile>]    Sip trace (optional profile to trace on, can be used multiple times)
      -sd --sip-debug=<level>        Set SIP debug level
      -oa --obfuscate-auto           Auto obfuscate sensitive information (ips/passwords/hashes/domains)
      -of --obfuscate-file=<file>    File containing strings to obfuscate from the log (one per line, can use regexp if line starts with ^)
                                         if line contains an equals sign(not proceeded by a '\\') what is to the right of the equals sign is used as the replacement
      -do --display-output           Display output on stdout
      -ia --input-accept             Pass input to the freeswitch console
      -D, --fslogger-debug           FSLogger debug mode

      fs_logger.pl will run until fs_cli ends or control+c
~;
	print STDERR $usage;
	exit;
}
sub dbg_exec_get($){
	my ($cmds) = @_;
	$cmds =~ s/\n(?=.)/\n\t\t/gs;
	return $cmds;
}
sub main(){
	parse_args();
	my $to_write_fs_cli="";
	$CLEANUP_COMMANDS="";
	foreach my $to_trace (@SIP_TRACE_ON){
		$to_write_fs_cli .= $to_trace eq "_global_" ? "sofia global siptrace on\n" : "sofia profile " . $to_trace . " siptrace on\n";
		$CLEANUP_COMMANDS .= $to_trace eq "_global_" ? "sofia global siptrace off\n" : "sofia profile " . $to_trace . " siptrace off\n";
	}
	$to_write_fs_cli .= "sofia loglevel all $SOFIA_LOG_LEVEL\n" if ($SOFIA_LOG_LEVEL);
	$CLEANUP_COMMANDS .= "sofia loglevel all 0\n" if ($SOFIA_LOG_LEVEL);

	foreach my $cmd (@EXEC_ON_CONNECT){
		$to_write_fs_cli .= $cmd . "\n";
	}
	foreach my $cmd (@EXEC_ON_QUIT){
		$CLEANUP_COMMANDS .= $cmd . "\n";
	}
	unshift @CMD, $FS_CLI;
	print STDERR "Going to run: \"" . join(",",@CMD) . "\"\n\tand on execute:" . dbg_exec_get("\n" . $to_write_fs_cli) . "\n\tand on quit:" . dbg_exec_get("\n" . $CLEANUP_COMMANDS) . "\n" if ($DEBUG_MODE);
	my ($proc_stdout,$stdin_socket);
	($pid,$proc_stdout,$proc_stdin) = exec_with_socket(@CMD);
	my $vi = '';
	my $fn_proc_stdout = fileno $proc_stdout;
	vec($vi, $fn_proc_stdout, 1) = 1;
	print STDERR "Ran application it has pid: $pid and stdout fn#: " . $fn_proc_stdout . ", will wait to see if it quit right away\n" if ($DEBUG_MODE);
	if ( select(my $vin=$vi, undef, undef, 0.5) > 0) { #we need to do this right away, if we try to do anything else we won't catch the error output if it terminates instantly
		sysread($proc_stdout, my $read_buffer, 2048);
		print $read_buffer if ($DISPLAY_OUTPUT);
		$output_buffer .= $read_buffer;
	}
	if ($ACCEPT_INPUT){
		print STDERR "Going to open stdin to a socket so we can accept input from user\n" if ($DEBUG_MODE);
		open($stdin_socket, "<&STDIN") if (! $IS_WINDOWS);
		$stdin_socket = socket_stdin() if ($IS_WINDOWS);
	} else {
		close (STDIN);
	}
	my $fn_proc_stdin = fileno $proc_stdin;
	my $fn_stdin= $ACCEPT_INPUT ? fileno $stdin_socket : 0;
	vec($vi, $fn_stdin, 1) = 1 if ($ACCEPT_INPUT);
	my $vo = '';
	vec($vo, $fn_proc_stdin, 1) = 1;
	my $vo_now = $to_write_fs_cli ? $vo : undef;
	my $true=1;
	print STDERR "Process stdin fn# $fn_proc_stdin and if we are accepting input from stdin its fn# $fn_stdin, going to check to see if our app already terminated\n" if ($DEBUG_MODE);
	$true = 0 if (waitpid($pid, WNOHANG) != 0); #terminated instantly
	print STDERR "Going to enter main event loop\n" if ($DEBUG_MODE && $true);
	my $start_time=time;
	while ($true) {
		if ( select(my $vin=$vi, my $von=$vo_now, undef, 0.5) > 0) {
			last if ( waitpid($pid, WNOHANG) != 0); #application terminated
			if (vec($vin, $fn_proc_stdout, 1)) {
				sysread($proc_stdout, my $read_buffer, 2048) or last;
				print $read_buffer if ($DISPLAY_OUTPUT);
				$output_buffer .= $read_buffer;
			}
			if ($vo_now && vec($von, $fn_proc_stdin, 1)) {
				my $wrote = syswrite ($proc_stdin, $to_write_fs_cli);
				$to_write_fs_cli = substr($to_write_fs_cli, $wrote);
				$vo_now = undef if (! length($to_write_fs_cli));
				print STDERR "Wrote to proc_stdin of length: $wrote\n" if ($DEBUG_MODE);
			}
			if ($ACCEPT_INPUT && vec($vin, $fn_stdin, 1)) {
				sysread($stdin_socket, my $read_buffer, 2048);
				print STDERR "Read input from STDIN of: $read_buffer\n" if ($DEBUG_MODE);
				if ($read_buffer =~ /^(\.\.\.|\/quit|\/exit|\/bye)$/m){ #lets intercept quit so we can cleanup properly
					cleanup(0);
					return;
				}
				$to_write_fs_cli .= $read_buffer;
				$vo_now = $vo;
			}
		} else {
			last if ( waitpid($pid, WNOHANG) != 0);
		}
		#print ".";
	}
	my $secs = time - $start_time;
	if ($secs < 3){
		print $output_buffer if (! $DISPLAY_OUTPUT); #tell them what we got back if they didnt see it before due to not showing output
		print STDERR "fs_cli terminated on start!\n";
	}
	cleanup(1);
}

sub cleanup{
	my ($unexpected) = @_;
	$unexpected = "" if ($unexpected ne "1");
	print STDERR "Going to cleanup fs_cli is most likely " . ($unexpected ? "not " : "") . "still running\n" if ($DEBUG_MODE);
	return if ($in_cleanup);
	$in_cleanup = 1;
	if ($CLEANUP_COMMANDS && ! $unexpected){
		syswrite($proc_stdin,"\n" . $CLEANUP_COMMANDS . "...\n");
		my $max_wait = 8;
		for (my $x = 0; $x < $max_wait; $x++){
			last if ( waitpid($pid, WNOHANG) != 0);
			sleep(0.1);
			print STDERR "Waiting for fs_cli to finish: $x\n" if ($DEBUG_MODE);
		}
	}
	kill 9, $pid if ($pid);
	obfuscate_buffer();
	print "\n" . ($unexpected ? "OK" : "Good")  . " Bye!\n";
	print "\n" . pastebin_post($PASTEBIN_USER,$output_buffer) . "\n" if ($PASTEBIN_USER);
	puke($FILE,$output_buffer) if ($FILE);
	exit;
}
sub udp_socket_pair() {
	my $in = IO::Socket::INET->new( LocalAddr => 'localhost', Proto => "udp", LocalPort => 0) or die "Failed to open port";
#    my $true = 1;
#    ioctl( $sock, 0x8004667e, \$true ) or die $!; #not needed atleast in modern windows/linux it seems
	my $out = IO::Socket::INET->new( PeerAddr => 'localhost', Proto=> "udp", PeerPort => $in->sockport() ) or die "Failed to bind remote port " . $in->sockport();
	return ($in,$out);
}
sub socket_stdin() { #for some reason socketpair doesn't seem to work cross threads properly so use good old sockets.
	my ($in,$out) = udp_socket_pair();
	threads::async {
		print $out $_ while <STDIN>;
	}->detach;
	return $in;
}
sub exec_with_socket(@){#Only way I have been able to open a nonblocking on read process in windows
	my (@CMD) = @_;
	my ($pid, $oldin, $oldout, $olderr);
	my ($server, $client);
	socketpair($server, $client, AF_UNIX, SOCK_STREAM, PF_UNSPEC) or  die "socket pair: $!";
	if (! $IS_WINDOWS){
		my $pid = fork();
		if ($pid == 0){
			if (open (STDIN, '<&', $server) and open (STDOUT, '>&', $server) and open (STDERR, '>&', $server)) {
				exec(@CMD) or die "exec failed: $!";
			}
			die "error opening inputs/outputs";
		} else {
			return ($pid and $pid > 0) ? ($pid, $client, $client) : ();
		}
	} else {
		open $oldin, '<&', \*STDIN or die "error opening STDIN";
		open $oldout, '>&', \*STDOUT or die "error opening STDOUT";
		open $olderr, '>&', \*STDERR or die "error opening STDERR";
		my ($stdin_in,$stdin_out) = udp_socket_pair(); #for some reason tieing server to STDIN for some reason doesn't work, so here is the fix
		print $stdin_out "\n";#have to prime it for some reason so need something to grab when it goes to read
		open (STDIN, '<&', $stdin_in) or die "Unable to open stdin";
		my $primer = <STDIN>; #required to get the child to start reading...
		if ( open (STDOUT, '>&', $server) and open (STDERR, '>&', $server)) {
			$pid = eval { system 1, @_ or die "exec failed: $!"};
		}

		close STDERR;
		open STDERR, '>&', $olderr or die "error restoring STDERR";
		close STDOUT;
		open STDOUT, '>&', $oldout or die "error restoring STDOUT";
		close STDIN;
		open STDIN, '<&', $oldin or die "error restoring STDIN";
		return ($pid and $pid > 0) ? ($pid, $client, $stdin_out) : ();
	}
}
sub pastebin_post($$){
	my ($post_as,$to_post) = @_;
	$post_as = uri_escape($post_as);
	$to_post = uri_escape($to_post);
	my $pb_post = qq~POST http://pastebin.freeswitch.com/pastebin.php HTTP/1.1
	Accept: text/html, application/xhtml+xml, */*
	Referer: http://pastebin.freeswitch.com/
	Accept-Language: en-US
	User-Agent: FSLogger
	Content-Type: application/x-www-form-urlencoded
	Accept-Encoding: gzip, deflate
	Host: pastebin.freeswitch.com
	Content-Length: CONT_LEN
	Pragma: no-cache
	Authorization: Basic cGFzdGViaW46ZnJlZXN3aXRjaA==

	~;
	$pb_post =~ s/^\t//mg;
	my $post_body = "parent_pid=&format=fslog&poster=" . $post_as . "&paste=Send&expiry=m&code2=";
	$post_body .= $to_post;
	my $post_len = length($post_body);
	$pb_post =~ s/CONT_LEN/$post_len/;
	$pb_post .= $post_body;
	my $sock = new IO::Socket::INET ( PeerAddr => 'pastebin.freeswitch.org', PeerPort => '80', Proto => 'tcp', );
	print $sock $pb_post;
	my $url="";
	while (my $line = <$sock>){
		$line =~ s/\r//gs;
		chomp($line);
		($url = $1) && last if ($line =~ /^Location: (.+)/);
		last if ($line eq "");
	}
	close $sock;
	return $url;
}
sub strip_quotes($){
	my ($str) = @_;
	$str =~ s/^"//;
	$str =~ s/"$//;
	return $str;
}
sub str_replace($$$){
	my ($string, $to_replace, $replace_with) = @_;
	my $pos = 0;
	my $to_replace_len = length($to_replace);
	return $string if ($to_replace_len == 0);
	my $replace_with_len = length($replace_with);

	while ( ($pos = index($string, $to_replace, $pos)) != -1 ) {
		 substr($string, $pos, $to_replace_len) = $replace_with;
		 $pos += $replace_with_len;
	}
	return $string;
}
{
	# http://cpansearch.perl.org/src/GAAS/URI-1.58/URI/Escape.pm
	my %subst;
	my %escapes;
	for (0..255) {
	    $escapes{chr($_)} = sprintf("%%%02X", $_);
	}
	my %Unsafe = (
	    RFC2732 => qr/[^A-Za-z0-9\-_.!~*'()]/,
	    RFC3986 => qr/[^A-Za-z0-9\-\._~]/,
	);
	sub _fail_hi {
	    my $chr = shift;
		die(sprintf "Can't escape \\x{%04X}, try uri_escape_utf8() instead", ord($chr));
	}
	sub uri_escape {
		my($text, $patn) = @_;
		return undef unless defined $text;
		if (defined $patn){
			unless (exists  $subst{$patn}) {
				# Because we can't compile the regex we fake it with a cached sub
				(my $tmp = $patn) =~ s,/,\\/,g;
				eval "\$subst{\$patn} = sub {\$_[0] =~ s/([$tmp])/\$escapes{\$1} || _fail_hi(\$1)/ge; }";
				die("uri_escape: $@") if $@;
			}
			&{$subst{$patn}}($text);
		} else {
			$text =~ s/($Unsafe{RFC3986})/$escapes{$1} || d($1)/ge;
		}
		$text;
	}
}
sub slurp($){
	my ($file) = @_;
	my $cont;
	open(my $hdl, $file);
	{
		local $/ = undef;
		$cont = <$hdl>;
	}
	close ($hdl);
	return $cont;
}
sub puke($$){
	my ($file,$cont) = @_;
	open(my $hdl,">" . $file);
	flock $hdl, 2;
	print $hdl $cont;
	close ($hdl);
}
######################COMMAND LINE ARG PARSING####################
{
	my $parse_pos;
	my $ignore_if_already=0;
	my $argc = @ARGV;
	my %CMDS_DONE;
	sub arg_test($$$$){ #returns if_match,value
		my ($short,$long,$has_value,$must_have_value) = @_;
		my $force_no_match=0;
		if ($ARGV[$parse_pos] eq $short) {
			die "option: $short already specified" if ($CMDS_DONE{$short} && ($force_no_match=1) && ! $ignore_if_already); #yes i want to do assignemnt
			$CMDS_DONE{$short} = 1;
			return (1 && ! $force_no_match,"") if (! $has_value);
			if ($ARGV[$parse_pos+1] eq "" || $ARGV[$parse_pos+1] =~ /^\-/){
				die "$short must be proceeded by a valid value"  if ($must_have_value);
				return (1 && ! $force_no_match,"");
			}
			$parse_pos++;
			return (1 && ! $force_no_match,strip_quotes($ARGV[$parse_pos]));
		} elsif($long && index($ARGV[$parse_pos],$long) == 0){
			die "option: $long already specified" if ($CMDS_DONE{$short} && ($force_no_match=1) && ! $ignore_if_already); #yes i want to do assignemnt
			$CMDS_DONE{$short} = 1;
			return (1 && ! $force_no_match,"") if (! $has_value);
			if ($ARGV[$parse_pos] =~ /\s*=\s*([^\s]+)/){
				return (1 && ! $force_no_match,$1);
			}
			if ($ARGV[$parse_pos+1] eq "" || $ARGV[$parse_pos+1] =~ /^\-/){
				die "$long must be proceeded by a valid value"  if ($must_have_value);
				return (1 && ! $force_no_match,"");
			}
			$parse_pos++;
			return (1 && ! $force_no_match, strip_quotes($ARGV[$parse_pos]));
		}
		return (0,"");
	}
	sub parse_args_through{
		my ($short,$long,$through_cmd,$no_arg) = @_;
		$through_cmd = $short if (! $through_cmd);
		my ($matches,$value) = arg_test($short,$long,! $no_arg,! $no_arg);
		if ($matches){
			push @CMD, $short, $value;
			return 1;
		}
		return 0;
	}
	sub parse_args(){
		my $do_auto = 0;
		for($parse_pos = 0; $parse_pos < $argc; $parse_pos++){
			my ($matches,$value);
			($matches,$value) = arg_test("-h","--help",0,0);
			usage() if ($matches);
			next if (parse_args_through("-H","--host"));
			next if (parse_args_through("-P","--port"));
			next if (parse_args_through("-u","--user"));
			next if (parse_args_through("-p","--password"));
			next if (parse_args_through("-l","--loglevel"));
			next if (parse_args_through("-d","--debug"));
			next if (parse_args_through("-q","--quiet","",1));
			next if (parse_args_through("-r","--retry","",1));
			next if (parse_args_through("-R","--reconnect","",1));
			($matches,$value) = arg_test("-A","--auto",0,0);
			$do_auto = 1 and next if ($matches);
			($matches,$value) = arg_test("-x","--execute",1,1);
			if ($matches){
				push @EXEC_ON_CONNECT, $value;
				$CMDS_DONE{"-x"} = undef;
				next;
			}
			($matches,$value) = arg_test("-X","--quit-execute",1,1);
			if ($matches){
				push @EXEC_ON_QUIT, $value;
				$CMDS_DONE{"-X"} = undef;
				next;
			}
			($matches,$value) = arg_test("-f","--file",1,1);
			$FILE = $value and next if ($matches);
			($matches,$value) = arg_test("-pb","--paste-bin",1,0);
			$PASTEBIN_USER = ($value ? $value : "FSLogger") and next if ($matches);

			($matches,$value) = arg_test("-st","--sip-trace",1,0);
			if ($matches){
				push @SIP_TRACE_ON, $value eq "" ? "_global_" : $value;
				$CMDS_DONE{"-st"} = undef;
				next;
			}
			($matches,$value) = arg_test("-sd","--sip-debug",1,1);
			if ($matches){
				$SOFIA_LOG_LEVEL = $value;
				next;
			}
			($matches,$value) = arg_test("-oa","--obfuscate-auto",0,0);
			$OB_AUTO=1 and next if ($matches);
			($matches,$value) = arg_test("-D","--fslogger-debug",0,0);
			$DEBUG_MODE=1 and next if ($matches);
			($matches,$value) = arg_test("-of","--obfuscate-file",1,1);
			die "Obfuscate file not found: $value" if ($matches && ! -e $value);
			$OB_FILE=$value and next if ($matches);
			($matches,$value) = arg_test("-do","--display-output",0,0);
			$DISPLAY_OUTPUT=1 and next if ($matches);
			($matches,$value) = arg_test("-ia","--input-accept",0,0);
			die "Sorry you perl doesn't threading and its required for input redirection on windows" if ($matches && $IS_WINDOWS && ! $THREADS_SUPPORTED);
			$ACCEPT_INPUT=1 and next if ($matches);

			next if ($ignore_if_already);			
			print STDERR "Invalid option: " . $ARGV[$parse_pos] . "\n";
			usage();
		}
		if ($do_auto){
			$ignore_if_already=1;
			@ARGV = @AUTOS;
			$argc = @ARGV;
			$CMDS_DONE{"-st"} = 1 if (@SIP_TRACE_ON);
			parse_args();
			foreach my $profile (@SIP_TRACE_ON){
				next if ($profile ne "none");
				@SIP_TRACE_ON = ();
				last;
			}
		}
		if (! $FILE && ! $PASTEBIN_USER){
			print STDERR "One of: -A(--auto) or -f(--file) or -pb(--paste-bin) must be specified\n";
			usage();
		}
	}
}
###################END COMMAND LINE ARG PARSING###################


##############################Obfuscation code######################################
#Note the obfuscator is not perfect, if you have sensitive data that would also normally come up in the log it will give away the sensitive data (IE if your password is set to 5060 for an account all instances of 5060 are replaced with a random password, so don't user super simple sensitive data...)
{
	my $IP_REGEX = qr/(?<![.0-9])([0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3})(?![.0-9])/; #negatives to avoid oids
	my $DOM_REGEX = qr/([a-z0-9\.\-_\+\=]+[a-z]\.(?:com|net|org|biz|gov|edu|mil|uk)(?![a-z]))/i;  #not a huge list but should help avoid false positives, can always stick your domain in the obs file to get ones missed
	my $NONCE_REGEX = qr/nonce="([^"]+)"/i;
	my $MD5AUTH_REGEX = qr/MD5\([^:]+:[^:]+:([^:]+)\)\s*=\s*([a-z0-9]+)/i;  #capture both the unencrypted password and the resulting hash
	my $MD5_REGEX = qr/(?<![a-z0-9])([a-z0-9]{32,32})(?![a-z0-9])/i; #32 char letter and number only strings
	my $MD5_RESP = qr/response="$MD5_REGEX"/;
	my %OB_TO_REPLACE;

	sub ob_rand_str($){
		my ($len) = @_;
		return join '', map { ('a'..'z','A'..'Z',0..9)[rand 62] } 0..$len;
	}
	{
		my %OB_IP_CACHE;
		sub ob_get_replacement_ip($){ #try to keep classes the same
			my ($ip) = @_;
			my (@parts) = split(/\./,$ip);
			my $to_add=0;
			my @cur = @parts;
			my $obs = "";
			for (my $x = 4; $x > 0; $x--){
				last if ($OB_IP_CACHE{join '.',@cur});
				splice @cur, $x-1, 1;
			}
			if (@cur){
				$obs = $OB_IP_CACHE{join '.',@cur};
			}

			for (my $x = @cur; $x < 4; $x++){
				my $next;
				while(1){ #to avoid possibly looping forever (as we don't return 0 ips) we allow for the impossible 256.
					$next = int(rand(255))+1;
					last if (! $OB_IP_CACHE{join '.',@cur,$next});
				}
				push @cur, $parts[$x];
				$obs .= "." if ($obs);
				$obs .= $next;
				$OB_IP_CACHE{join '.',@cur}=$obs;
			}
			return $obs;
		}
	}
	{
		my $dom_cntr=100;
		sub ob_get_replacement_dom($){
			#return "ob" . lc(ob_rand_str(8)) . ".com";
			return "freeswitch" . $dom_cntr++ . ".com"; #easier to read and remember
		}
	}
	sub ob_get_replacement_str($){
		my ($inp) = @_;
		return ob_get_replacement_ip($inp) if ($inp =~ /$IP_REGEX/);
		return ob_get_replacement_dom($inp) if ($inp =~ /$DOM_REGEX/);
		return "OB" . lc(ob_rand_str(30)) if ($inp =~ /$MD5_REGEX/);
		return "OB" . ob_rand_str(8);
	}
	sub ob_handle_found(@){
		my (@arr) = @_;
		foreach my $item (@arr){
			next if ($OB_TO_REPLACE{$item});
			$OB_TO_REPLACE{$item}=ob_get_replacement_str($item);
		}
	}
	sub obfuscate_buffer(){
		if ($OB_FILE){
			my $cont = slurp($OB_FILE);
			$cont =~ s/\r//gs;
			my @lines = split("\n",$cont);
			foreach my $line (@lines){
				my $replace_with;
				if ($line =~ /(.+)(?<!\\)=(.+)/){
					$line = $1;
					$replace_with=$2;
				}
				if ($line =~/^\^(.+)/){
					my @matches=($output_buffer =~ m|$1|g);
					if ($replace_with){
						foreach my $match (@matches){
							$OB_TO_REPLACE{$match} = $replace_with;
						}
						next;
					}
					ob_handle_found(@matches);
					next;
				}
				$OB_TO_REPLACE{$line} = $replace_with ? $replace_with : ob_get_replacement_str($line);
			}
		}
		if ($OB_AUTO){
			ob_handle_found($output_buffer =~ m|$IP_REGEX|g);
			ob_handle_found($output_buffer =~ m|$DOM_REGEX|g);
			ob_handle_found($output_buffer =~ m|$NONCE_REGEX|g);
			ob_handle_found($output_buffer =~ m|$MD5AUTH_REGEX|g);
			ob_handle_found($output_buffer =~ m|$MD5_RESP|g);
		}
		foreach my $item (sort { length($b) <=> length($a) } keys %OB_TO_REPLACE){
			$output_buffer = str_replace($output_buffer,$item,$OB_TO_REPLACE{$item});
		}
	}
}
############################End Obfuscation code####################################

main();