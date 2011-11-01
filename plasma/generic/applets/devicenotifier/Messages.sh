#! /usr/bin/env bash
$XGETTEXT `find . -name \*.qml` -L Java -o $podir/notifier.pot
rm -f rc.cpp
