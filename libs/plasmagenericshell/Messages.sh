#! /usr/bin/env bash
$EXTRACTRC *.ui >> rc.cpp
$XGETTEXT `find . -name \*.cpp -o -name \*.h -o -name \*.qml | grep -v '/tests/'` -o $podir/plasmagenericshell.pot
rm -f rc.cpp
