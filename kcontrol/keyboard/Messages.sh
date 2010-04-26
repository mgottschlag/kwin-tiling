#! /usr/bin/env bash
$EXTRACTRC kcmmiscwidget.ui >> rc.cpp
$XGETTEXT rc.cpp kcmmisc.cpp -o $podir/kcmkeyboard.pot
rm -f rc.cpp
$EXTRACTRC kcm_*.ui >> rc.cpp
$XGETTEXT kcm_*.cpp keyboard_*.cpp -o $podir/kxkb.pot
rm -f rc.cpp
