#! /usr/bin/env bash
$EXTRACTRC `find . -name \*.kcfg` >> rc.cpp
$XGETTEXT *.cpp -o $podir/plasma-desktop.pot
rm -f rc.cpp
