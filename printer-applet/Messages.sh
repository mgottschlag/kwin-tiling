#!/bin/sh
$EXTRACTRC *.rc *.ui >> ./rc.py
$XGETTEXT --language=Python *.py -o $podir/printer-applet.pot
rm -f rc.py
