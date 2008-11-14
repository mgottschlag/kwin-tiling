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
        self.importer = plasma_importer.PlasmaImporter()
        self.initialized = False

    def init(self):
        print("Script Name: " + self.dataEngine().name())

        self.m_moduleName = str(self.dataEngine().pluginName())
        print("pluginname: " + str(self.dataEngine().pluginName()))
        self.plugin_name = str(self.dataEngine().pluginName()).replace('-','_')

        self.importer.register_top_level(self.plugin_name, str(self.dataEngine().package().path()))

        print("mainscript: " + str(self.dataEngine().package().filePath("mainscript")))
        print("package path: " + str(self.dataEngine().package().path()))

        # import the code at the file name reported by mainScript()
        self.module = __import__(self.plugin_name+'.main')
        self.pydataengine = self.module.main.CreateDataEngine(None)
        #self.pydataengine.setDataEngine(self.dataEngine())
        self.pydataengine.setDataEngineScript(self)
        self.pydataengine.init()

        self.initialized = True
        return True

    def __dtor__(self):
        print("~PythonDataEngineScript()")
        PythonAppletScript.importer.unregister_top_level(self.plugin_name)
        self.pydataengine = None

    def sources(self):
        return self.pydataengine.sources()

    def serviceForSource(self,source):
        return self.pydataengine.serviceForSource(source)

    def sourceRequestEvent(self,name):
        return self.pydataengine.sourceRequestEvent(name)

    def updateSourceEvent(self,source):
        return self.pydataengine.updateSourceEvent(source)

def CreatePlugin(widget_parent, parent, component_data):
    return PythonDataEngineScript(parent)
