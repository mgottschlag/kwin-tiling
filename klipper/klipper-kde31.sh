#!/bin/sh
filename=`kde-config --localprefix`share/autostart/klipper.desktop
if grep 'Hidden=true' "$filename" > /dev/null 2> /dev/null; then
  echo AutoStart=false
else
  echo AutoStart=true
fi
rm -f "$filename"