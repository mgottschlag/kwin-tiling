#! /usr/bin/env bash
sed 's/X-KDE-Category=\(.*\)/i18nc\("Screen saver category","\1"\);/' < category_list > category_list.cpp
$XGETTEXT *.cpp -o $podir/kcmscreensaver.pot
rm -f category_list.cpp
