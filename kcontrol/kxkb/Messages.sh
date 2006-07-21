#! /usr/bin/env bash
$XGETTEXT kxkb.cpp extension.cpp rc.cpp -o $podir/kxkb.pot
$XGETTEXT kcmlayout.cpp kcmmisc.cpp rc.cpp -o $podir/kcmlayout.pot
$XGETTEXT rules.cpp pixmap.cpp kxkbbindings.cpp rc.cpp -o $podir/kxkb_common.pot
