#! /bin/sh
# Translation of tiles is used by lookandfeeltab_impl.cpp
# Tile names are transformed to words with title case
### FIXME: Where are the tiles?
true || (cd ../../../kicker/data/tiles ; ls *_tiny_up.png) | perl -p -e \
's/(.*)_tiny_up\.png/i18n\(\"\u$1\"\)\;/; s/[_ ]+(.)/ \u$1/g' >> rc.cpp
$XGETTEXT *.cpp -o $podir/kcmkicker.pot
