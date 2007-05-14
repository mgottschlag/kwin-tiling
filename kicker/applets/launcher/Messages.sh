#! /usr/bin/env bash
$EXTRACTRC *.rc *.ui *.kcfg > rc.cpp
$XGETTEXT *.cpp *.h -o $podir/quicklauncher.pot
