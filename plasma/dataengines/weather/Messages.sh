#! /usr/bin/env bash
grep "weather condition" ions/data/envcan_i18n.dat | awk -F'|' '{ printf("i18nc(\"%s\", \"%s\");\n", $1, $2) }' >> rc.cpp
grep "weather forecast" ions/data/envcan_i18n.dat | awk -F'|' '{ printf("i18nc(\"%s\", \"%s\");\n", $1, $2) }' >> rc.cpp
grep "wind direction" ions/data/envcan_i18n.dat | awk -F'|' '{ printf("i18nc(\"%s\", \"%s\");\n", $1, $2) }' >> rc.cpp
grep "wind speed" ions/data/envcan_i18n.dat | awk -F'|' '{ printf("i18nc(\"%s\", \"%s\");\n", $1, $2) }' >> rc.cpp

grep "weather condition" ions/data/noaa_i18n.dat | awk -F'|' '{ printf("i18nc(\"%s\", \"%s\");\n", $1, $2) }' >> rc.cpp
grep "weather forecast" ions/data/noaa_i18n.dat | awk -F'|' '{ printf("i18nc(\"%s\", \"%s\");\n", $1, $2) }' >> rc.cpp
grep "wind direction" ions/data/noaa_i18n.dat | awk -F'|' '{ printf("i18nc(\"%s\", \"%s\");\n", $1, $2) }' >> rc.cpp
grep "wind speed" ions/data/envcan_i18n.dat | awk -F'|' '{ printf("i18nc(\"%s\", \"%s\");\n", $1, $2) }' >> rc.cpp

grep "weather condition" ions/data/bbcukmet_i18n.dat | awk -F'|' '{ printf("i18nc(\"%s\", \"%s\");\n", $1, $2) }' >> rc.cpp
grep "weather forecast" ions/data/bbcukmet_i18n.dat | awk -F'|' '{ printf("i18nc(\"%s\", \"%s\");\n", $1, $2) }' >> rc.cpp
grep "wind direction" ions/data/bbcukmet_i18n.dat | awk -F'|' '{ printf("i18nc(\"%s\", \"%s\");\n", $1, $2) }' >> rc.cpp


$XGETTEXT `find . -name \*.cpp` -o $podir/plasma_engine_weather.pot
