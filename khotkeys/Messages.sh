#! /usr/bin/env bash
$EXTRACTRC kcontrol/ui/*.ui data/*.khotkeys >> rc.cpp || exit 11
$XGETTEXT rc.cpp app/*.cpp shared/*.cpp shared/*.h kcontrol/*.cpp -o $podir/khotkeys.pot
