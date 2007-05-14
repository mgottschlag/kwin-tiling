#! /usr/bin/env bash
$EXTRACTRC *.rc *.ui *.kcfg > rc.cpp
$XGETTEXT *.cc *.cpp *.h -o $podir/kdesktop.pot
