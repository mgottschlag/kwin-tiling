#! /usr/bin/env bash
$EXTRACTRC *.rc *.ui *.kcfg > rc.cpp
$XGETTEXT *.cpp -o $podir/kcmsmartcard.pot
