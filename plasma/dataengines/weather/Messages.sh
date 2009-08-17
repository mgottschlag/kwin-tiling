#! /usr/bin/env bash
for file in ions/data/*.dat
do
  grep '|' $file | awk -F'|' '{ print "// i18n: file: '`basename $file`'"; printf("i18nc(\"%s\", \"%s\");\n", $1, $2) }' >> rc.cpp
done

$XGETTEXT `find . -name \*.cpp` -o $podir/plasma_engine_weather.pot
