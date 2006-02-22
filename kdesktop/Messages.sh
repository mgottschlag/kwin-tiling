#! /usr/bin/env bash
$EXTRACTRC lock/*.ui >> rc.cpp || exit 11
$XGETTEXT lock/*.cc *.cc *.cpp *.h -o $podir/kdesktop.pot
