#!/usr/bin/perl

my ($section, %data);

#read in all the data, split it up into hashes. Thanks again to malte for much input
while (<>) {
	if (/\[(.*)\]/) {
		$sections{$section} = {%data} if $section;
		$section = $1;
		undef %data;
		next;
 	}
	$data{$1} = $2 if /^([^=]*)=(.*)$/;
}

$sections{$section} = {%data} if $section;

$numActions = $sections{'General'}->{'Number of Actions'};
for my $i (0..($numActions - 1)) {
	my $actionGroup = "Action_$i";
	my $numCommands = $sections{$actionGroup}->{'Number of commands'};

	print "[$actionGroup]\n";
	# rename some keys
	print "Description=$sections{$actionGroup}->{'Action description'}\n";
	print "Regexp=$sections{$actionGroup}->{'Action regexp'}\n";
	print "Number of commands=$numCommands\n";

	# move the command entries from "Action_x" to "Action_x/Command_y"
	for my $k (0..($numCommands - 1)) {
		my $command = "Command_$k";
		my $commandGroup = "$actionGroup/$command";
		print "[$commandGroup]\n";
		my $value = $sections{$actionGroup}->{"$command: commandline"};
		print "Commandline=$value\n";
		$value = $sections{$actionGroup}->{"$command: description"};
		print "Description=$value\n";
		$value = $sections{$actionGroup}->{"$command: enabled"};
		print "Enabled=$value\n";
	}
}

