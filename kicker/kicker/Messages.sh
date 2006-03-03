#! /usr/bin/env bash
$EXTRACTRC ui/*ui >> rc.cpp || exit 11
$EXTRACTRC core/*.kcfg >> rc.cpp || exit 12
$XGETTEXT buttons/*.cpp core/*.cpp ui/*.cpp *.cpp -o $podir/kicker.pot
