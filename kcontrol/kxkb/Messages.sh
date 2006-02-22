#! /usr/bin/env bash
$XGETTEXT kxkb.cpp rules.cpp extension.cpp pixmap.cpp kxkbbindings.cpp rc.cpp -o $podir/kxkb.pot
$XGETTEXT rules.cpp kcmlayout.cpp pixmap.cpp kcmmisc.cpp rc.cpp kxkbbindings.cpp -o $podir/kcmlayout.pot 
