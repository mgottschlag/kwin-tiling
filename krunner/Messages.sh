#! /usr/bin/env bash
$EXTRACTRC kcfg/*.kcfg *.ui >> rc.cpp
$XGETTEXT `find . -name \*.cpp -o -name \*.h -o -name \*.cc` -o $podir/krunner.pot
rm -f rc.cpp
