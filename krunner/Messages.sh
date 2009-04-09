#! /usr/bin/env bash
$EXTRACTRC kcfg/*.kcfg >> rc.cpp
$XGETTEXT `find . -name \*.cpp -o -name \*.cc` -o $podir/krunner.pot
rm -f rc.cpp
