#!/usr/bin/perl
$keys=0;
foreach (<>) {
    if(/^\[.*\]/) { $keys=0; }
    if($keys==1) {
        ($oldkey) = ($_ =~ /([^=]*)=.*/);
        s/^Execute Command/Run Command/;
        s/^Execute command/Run Command/;
        s/^Lock screen/Lock Session/;
        s/^Lock Screen/Lock Session/;
        s/^Mouse emulation/Mouse Emulation/;
        s/^Next keyboard layout/Switch to Next Keyboard Layout/;
        s/Switch To Next Keyboard Layout/Switch to Next Keyboard Layout/;
        s/^Screenshot of desktop/Desktop Screenshot/;
        s/^Pop-up window operations menu/Window Operations Menu/;
        s/^toggle-clipboard-actions/Enable\/Disable Clipboard Actions/;
        s/^Screenshot of active window/Window Screenshot/;
        s/^Show taskmanager/Show Taskmanager/;
        s/^Show window list/Show Window List/;
        s/^show-klipper-popupmenu/Show Klipper Popup-Menu/;
        s/^Switch desktop left/Switch One Desktop to the Left/;
        s/^Switch desktop right/Switch One Desktop to the Right/;
        s/^Switch desktop down/Switch One Desktop Down/;
        s/^Switch desktop up/Switch One Desktop Up/;
        s/^Switch to desktop /Switch to Desktop /;
        s/^Switch desktop next/Switch to Next Desktop/;
        s/^Switch desktop previous/Switch to Previous Desktop/;
        s/^Toggle Show Desktop/Toggle Showing Desktop/;
        s/^Toggle raise and lower/Toggle Window Raise\/Lower/;
        s/^Walk back through desktop list/Walk Through Desktop List \(Reverse\)/;
        s/^Walk back through desktops/Walk Through Desktops \(Reverse\)/;
        s/^Walk back through windows/Walk Through Windows \(Reverse\)/;
        s/^Walk through desktop list/Walk Through Desktop List/;
        s/^Walk through desktops/Walk Through Desktops/;
        s/^Walk through windows/Walk Through Windows/;
        s/^Window close/Window Close/;
        s/^Window iconify/Window Iconify/;
        s/^Window lower/Window Lower/;
        s/^Window maximize/Window Maximize/;
        s/^Window maximize horizontal/Window Maximize Horizontal/;
        s/^Window maximize vertical/Window Maximize Vertical/;
        s/^Window move/Window Move/;
        s/^Window raise/Window Raise/;
        s/^Window resize/Window Resize/;
        s/^Window shade/Window Shade/;
        s/^Window to Desktop /Window to Desktop /;
        s/^Window to next desktop/Window to Next Desktop/;
        s/^Window to previous desktop/Window to Previous Desktop/;
        s/^repeat-last-klipper-action/Show Klipper Popup-Menu/;
        ($newkey) = ($_ =~ /([^=]*)=.*/);
        if ($oldkey ne $newkey) {
            print "# DELETE " . $oldkey . "\n";
            print $_
        }
    }
    if(/\[Global Keys\]/) { $keys=1; print $_; }
    if(/\[Global Shortcuts\]/) { $keys=1; print $_; }
}
