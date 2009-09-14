#! /usr/bin/env bash
$EXTRACTRC *.ui >> rc.cpp
$XGETTEXT *.cpp -o $podir/plasma_appletscriptengine_qedjescripts.pot
rm -f rc.cpp
