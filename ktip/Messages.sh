#! /usr/bin/env bash
$PREPARETIPS > tips.cpp
$XGETTEXT *.cpp -o $podir/ktip.pot
rm -f tips.cpp
