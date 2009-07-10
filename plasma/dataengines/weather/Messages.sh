#! /usr/bin/env bash
sed -e 's/\([A-Za-z].*\)/i18nc("weather condition", "\1");/' ions/data/bbcukmet_i18n.dat >>rc.cpp
sed -e 's/\([A-Za-z].*\)/i18nc("weather condition", "\1");/' ions/data/envcan_i18n.dat >>rc.cpp
sed -e 's/\([A-Za-z].*\)/i18nc("weather condition", "\1");/' ions/data/noaa_i18n.dat >>rc.cpp
$XGETTEXT `find . -name \*.cpp` -o $podir/plasma_engine_weather.pot
