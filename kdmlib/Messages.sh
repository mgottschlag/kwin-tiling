#! /usr/bin/env bash
### TODO: why do we need 3 POT files for one directory?
$XGETTEXT kgreet_classic.cpp rc.cpp -o $podir/kgreet_classic.pot
$XGETTEXT kgreet_winbind.cpp rc.cpp -o $podir/kgreet_winbind.pot
$XGETTEXT dmctl.cpp rc.cpp -o $podir/libdmctl.pot
