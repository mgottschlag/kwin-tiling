#!/usr/bin/perl

while (<>)
{
    ($key, $value) = ($_ =~ /([^=]+)=[ \t]*([^\n]+)/);
    if ($key eq "GroupTasks")
    {
        if ($value eq "Never")
        {
            print "GroupTasks=GroupNever\n";
        }
        elsif ($value eq "When Taskbar Full")
        {
            print "GroupTasks=GroupWhenFull\n";
        }
        elsif ($value eq "Always")
        {
            print "GroupTasks=GroupAlways\n";
        }
    }
    elsif ($key =~ /ButtonAction/)
    {
        if ($value eq "Show Task List")
        {
            print "$key=ShowTaskList\n";
        }
        elsif ($value eq "Show Operations Menu")
        {
            print "$key=ShowOperationsMenu\n";
        }
        elsif ($value eq "Activate, Raise or Minimize Task")
        {
            print "$key=ActivateRaiseOrMinimize\n";
        }
        elsif ($value eq "Activate Task")
        {
            print "$key=Activate\n";
        }
        elsif ($value eq "Raise Task")
        {
            print "$key=Raise\n";
        }
        elsif ($value eq "Lower Task")
        {
            print "$key=Lower\n";
        }
        elsif ($value eq "Minimize Task")
        {
            print "$key=Minimize\n";
        }
        
    }
}

