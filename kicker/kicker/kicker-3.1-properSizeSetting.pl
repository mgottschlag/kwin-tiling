#!/usr/bin/perl

while (<>)
{
    ($key, $sizeSetting) = ($_ =~ /([^=]+)=[ \t]*([^\n]+)/);
    if ($key eq "Size")
    {
        print "[General]\n";

        if ($sizeSetting == '24')
        {
            print "Size=0\n";
        }
        elsif ($sizeSetting == '30')
        {
            print "Size=1\n";
        }
        elsif ($sizeSetting == '46')
        {
            print "Size=2\n";
        }
        elsif ($sizeSetting == '58')
        {
            print "Size=3\n";
        }
        elsif ($sizeSetting > 10)
        {
            print "Size=4\n";
            print "CustomSize=" . $sizeSetting . "\n";
        }
        exit(0);
    }
}

