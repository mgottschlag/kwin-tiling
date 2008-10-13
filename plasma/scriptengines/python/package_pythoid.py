#
# Copyright 2008 Simon Edwards <simon@simonzone.com>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU Library General Public License as
# published by the Free Software Foundation; either version 2, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details
#
# You should have received a copy of the GNU Library General Public
# License along with this program; if not, write to the
# Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
#

# Based on Richard Dale's package_ruboid.rb.

from PyKDE4.plasma import Plasma # Plasma C++ namespace
from PyKDE4.kdecore import *

class PackagePythoid(Plasma.PackageStructure):
    def __init__(self, parent, args=None):
        Plasma.PackageStructure.__init__(self, parent, "Pythoid")

        self.addDirectoryDefinition("images", "images", i18n("Images"))
        self.setMimetypes("images", ["image/svg+xml","image/png","image/jpeg"])

        self.addDirectoryDefinition("config", "config/", i18n("Configuration Definitions"))

        mimetypes = ["text/xml"]
        self.setMimetypes("config", mimetypes)
        self.setMimetypes("configui", mimetypes)

        self.addDirectoryDefinition("ui", "ui", i18n("Qt Designer UI files"))
        self.setMimetypes("ui", mimetypes)

        self.addDirectoryDefinition("scripts", "code", i18n("Executable Scripts"))
        
        mimetypes = ["text/*"]
        self.setMimetypes("scripts", mimetypes)

        self.addFileDefinition("mainconfiggui", "ui/config.ui", i18n("Main Config UI File"))
        self.addFileDefinition("mainconfigxml", "config/main.xml", i18n("Configuration XML file"))
        self.addFileDefinition("mainscript", "code/main.py", i18n("Main Script File"))
        self.setRequired("mainscript", True)

def CreatePlugin(widget_parent, parent, component_data):
    return PackagePythoid(parent)
