#!/usr/bin/perl
use FreeSWITCH::Client;
use Data::Dumper;

my $fs = init FreeSWITCH::Client {} or die "Error $@";
my $pid;

for (;;) {
  $fs->accept();

  if (!($pid = fork)) {
    last;
  }
}

my $data = $fs->call_data();

#print Dumper $data
print "Call: $data->{'caller-channel-name'} $data->{'unique-id'}\n";


$o = $fs->call_command("answer");
#to turn on events when in async mode
$o = $fs->raw_command("myevents");
$o = $fs->call_command("playback","/sounds/you-have-reached-a-test-number.wav");
$o = $fs->call_command("record","/tmp/recording.wav 5 300 5");
$o = $fs->call_command("playback","/tmp/recording.wav");
$o = $fs->call_command("echo");


#comment exit in async mode
#exit;

while(my $r = $fs->readhash(undef)) {
  if ($r->{socketerror}) {
    last;
  }

  if ($r->{has_event}) {
    print Dumper $r->{event};
  }
  if ($r->{event}->{'event-name'} !~ /execute/i) {
    printf "wtf $data->{'unique-id'}\n";
    $o = $fs->call_command("break");
    $o = $fs->call_command("hangup");
  }
}

$fs->disconnect();
print "done\n";


