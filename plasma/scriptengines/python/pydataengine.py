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
from PyQt4.QtCore import *
from PyQt4.QtGui import *
from PyKDE4.plasma import Plasma
import plasma_importer

class PythonDataEngineScript(Plasma.DataEngineScript):
    def __init__(self, parent):
        Plasma.DataEngineScript.__init__(self,parent)
        print("PythonDataEngineScript()")
        self.importer = plasma_importer.PlasmaImporter()
        self.initialized = False

    def init(self):
        print("Script Name: " + self.dataEngine().name()))
        
        print("Mainscript: " + self.mainScript())
        return False
        #self.pydataengine = klass.new(self)
        #self.pydataengine.init()
        #return True

    def sourceRequestEvent(self,name):
        self.pydataengine.sourceRequestEvent(name)

    def updateSourceEvent(self,source):
        self.pydataengine.updateSourceEvent(source)
