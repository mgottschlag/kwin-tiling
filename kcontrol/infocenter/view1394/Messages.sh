#! /usr/bin/env bash
$EXTRACTRC *.rc *.ui *.kcfg > rc.cpp
$XGETTEXT *.cpp -o $podir/kcmview1394.pot
