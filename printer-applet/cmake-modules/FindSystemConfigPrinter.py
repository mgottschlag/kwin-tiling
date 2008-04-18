# Copyright (c) 2008, Jonathan Riddell <jriddell@ubuntu.com>
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

import sys

SYSTEM_CONFIG_PRINTER_DIR = "/usr/share/system-config-printer"
sys.path.append (SYSTEM_CONFIG_PRINTER_DIR)

import ppds, cupshelpers

print "Groovy"
