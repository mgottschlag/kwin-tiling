#! /usr/bin/env bash
$EXTRACTRC *.ui >> rc.cpp
$XGETTEXT `find . -name \*.cpp -o -name \*.h | grep -v '/tests/'` -o $podir/plasmagenericshell.pot
rm -f rc.cpp
