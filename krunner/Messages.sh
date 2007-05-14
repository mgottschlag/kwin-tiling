#! /usr/bin/env bash
$XGETTEXT `find . -name \*.cpp -o -name \*.cc` -o $podir/krunner.pot
