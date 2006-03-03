#!/usr/bin/perl

$detailed = 1;
$namesfirst = 1;

while (<>)
{
    ($key, $value) = ($_ =~ /([^=]+)=[ \t]*([^\n]+)/);
    if ($key eq "DetailedMenuEntries")
    {
        if ($value eq "false")
        {
            $detailed = 0;
        }
    }
    elsif ($key eq "DetailedEntriesNamesFirst")
    {
        if ($value eq "false")
        {
            $namesfirst = 0;
        }
    }
}

if (not $detailed)
{
    print "MenuEntryFormat=NameOnly\n";
}
elsif ($namesfirst)
{
    print "MenuEntryFormat=NameAndDescription\n";
}
else
{
    print "MenuEntryFormat=DescriptionAndName\n";
}

