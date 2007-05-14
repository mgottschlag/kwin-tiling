#! /usr/bin/env bash
$EXTRACTRC *.rc *.ui *.kcfg > rc.cpp
$XGETTEXT *.cpp */*.cpp -o $podir/kcminput.pot
