#! /usr/bin/env bash
#grep "weather condition" envcan_i18n.dat | awk -F'|' '{ printf("i18nc(\"%s\", \"%s\");\n", $1, $2) }' >> rc.cpp
#grep "weather forecast" envcan_i18n.dat | awk -F'|' '{ printf("i18nc(\"%s\", \"%s\");\n", $1, $2) }' >> rc.cpp
#grep "wind direction" envcan_i18n.dat | awk -F'|' '{ printf("i18nc(\"%s\", \"%s\");\n", $1, $2) }' >> rc.cpp
#grep "wind speed" envcan_i18n.dat | awk -F'|' '{ printf("i18nc(\"%s\", \"%s\");\n", $1, $2) }' >> rc.cpp

#grep "weather condition" noaa_i18n.dat | awk -F'|' '{ printf("i18nc(\"%s\", \"%s\");\n", $1, $2) }' >> rc.cpp
#grep "weather forecast" noaa_i18n.dat | awk -F'|' '{ printf("i18nc(\"%s\", \"%s\");\n", $1, $2) }' >> rc.cpp
#grep "wind direction" noaa_i18n.dat | awk -F'|' '{ printf("i18nc(\"%s\", \"%s\");\n", $1, $2) }' >> rc.cpp
#grep "wind speed" envcan_i18n.dat | awk -F'|' '{ printf("i18nc(\"%s\", \"%s\");\n", $1, $2) }' >> rc.cpp


#grep "weather condition" bbcukmet_i18n.dat | awk -F'|' '{ printf("i18nc(\"%s\", \"%s\");\n", $1, $2) }' >> rc.cpp
#grep "weather forecast" bbcukmet_i18n.dat | awk -F'|' '{ printf("i18nc(\"%s\", \"%s\");\n", $1, $2) }' >> rc.cpp
#grep "wind direcption" bbcukmet_i18n.dat | awk -F'|' '{ printf("i18nc(\"%s\", \"%s\");\n", $1, $2) }' >> rc.cpp
#grep "wind speed" bbcukmet_i18n.dat | awk -F'|' '{ printf("i18nc(\"%s\", \"%s\");\n", $1, $2) }' >> rc.cpp


#$XGETTEXT `find . -name \*.cpp` -o $podir/plasma_engine_weather.pot
