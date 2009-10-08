#! /usr/bin/env bash
$EXTRACTRC `find . -name \*.ui` >> rc.cpp
$XGETTEXT *.cpp -o $podir/plasma_applet_devicenotifier.pot
