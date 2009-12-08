#! /usr/bin/env bash
$EXTRACTRC *.ui >> rc.cpp
$XGETTEXT *.cpp widgetsExplorer/*.cpp -o $podir/plasmagenericshell.pot
rm -f rc.cpp
