#! /usr/bin/env bash
$EXTRACTRC kcm*.ui >> rc.cpp
$XGETTEXT rc.cpp kcmmisc.cpp -o $podir/kcmkeyboard.pot
$XGETTEXT kcm_*.cpp keyboard_*.cpp layout_*.cpp flags.cpp layouts_menu.cpp -o $podir/kxkb.pot
rm -f rc.cpp
