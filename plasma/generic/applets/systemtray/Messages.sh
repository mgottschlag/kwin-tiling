#! /usr/bin/env bash
$EXTRACTRC ui/*.ui >> rc.cpp
$XGETTEXT notificationitemwatcher/*.cpp core/*.cpp rc.cpp ui/*.cpp -o $podir/plasma_applet_systemtray.pot
rm rc.cpp
