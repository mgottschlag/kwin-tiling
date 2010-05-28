#! /usr/bin/env bash
$EXTRACTRC `find . -name '*.ui'` >> rc.cpp
$XGETTEXT  `find . -name '*.cpp'` -o $podir/plasma_applet_notifications.pot
rm -f rc.cpp
