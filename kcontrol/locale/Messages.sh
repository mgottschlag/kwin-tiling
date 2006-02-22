#! /usr/bin/env bash
$XGETTEXT -ktranslate:1,1t -ktranslate:1c,2,2t *.cpp -o $podir/kcmlocale.pot
$XGETTEXT TIMEZONES rc.cpp -o $podir/../kdelibs/timezones.pot
