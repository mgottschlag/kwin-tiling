#! /usr/bin/env bash
$EXTRACTRC ui/*.ui >> rc.cpp
$XGETTEXT `find -iname \*.cpp` -o $podir/plasma_applet_systemtray.pot
rm rc.cpp
