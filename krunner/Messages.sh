#! /usr/bin/env bash
$EXTRACTRC *.rc *.ui *.kcfg > rc.cpp
$XGETTEXT `find . -name \*.cpp -o -name \*.cc` -o $podir/krunner.pot
