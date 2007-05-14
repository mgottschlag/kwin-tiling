#! /usr/bin/env bash
$EXTRACTRC *.rc *.ui *.kcfg > rc.cpp
sed 's/X-KDE-Category=\(.*\)/i18n\("Screen saver category","\1"\);/' < category_list > category_list.cpp
$XGETTEXT *.cpp -o $podir/kcmscreensaver.pot
rm -f category_list.cpp
