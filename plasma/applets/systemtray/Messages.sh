#! /usr/bin/env bash
$EXTRACTRC ui/*.ui >> rc.cpp
$XGETTEXT rc.cpp ui/*.cpp -o $podir/plasma_applet_systemtray.pot
rm rc.cpp
