#! /usr/bin/env bash
$EXTRACTRC kcm*.ui >> rc.cpp
$XGETTEXT rc.cpp kcmmisc.cpp -o $podir/kcmkeyboard.pot
$XGETTEXT kcm_*.cpp keyboard_*.cpp layout_widget.cpp flags.cpp -o $podir/kxkb.pot
rm -f rc.cpp
