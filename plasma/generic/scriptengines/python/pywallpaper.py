# -*- coding: utf-8 -*-
#
# Copyright 2008 Simon Edwards <simon@simonzone.com>
# Copyright 2009 Petri Damstén <damu@iki.fi>
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
import os.path

class PythonWallpaperScript(Plasma.WallpaperScript):
    importer = None

    def __init__(self, parent):
        Plasma.WallpaperScript.__init__(self,parent)
        if PythonWallpaperScript.importer is None:
            PythonWallpaperScript.importer = plasma_importer.PlasmaImporter()
        self.initialized = False

    def init(self):
        self.moduleName = str(self.wallpaper().pluginName())
        self.pluginName = 'wallpaper_' + self.moduleName.replace('-','_')

        PythonWallpaperScript.importer.register_top_level(self.pluginName, str(self.wallpaper().package().path()))

        # import the code at the file name reported by mainScript()
        relpath = os.path.relpath(str(self.mainScript()),str(self.wallpaper().package().path()))
        if relpath.startswith("contents/code/"):
            relpath = relpath[14:]
        if relpath.endswith(".py"):
            relpath = relpath[:-3]
        relpath = relpath.replace("/",".")
        self.module = __import__(self.pluginName+'.'+relpath)
        self.pywallpaper = self.module.main.CreateWallpaper(None)
        self.pywallpaper.setWallpaper(self.wallpaper())
        self.pywallpaper.setWallpaperScript(self)
        self.initialized = True
        return True

    def __dtor__(self):
        PythonWallpaperScript.importer.unregister_top_level(self.pluginName)
        self.pywallpaper = None

    def paint(self, painter, exposedRect):
        self.pywallpaper.paint(painter, exposedRect)

    def initWallpaper(self, config):
        self.pywallpaper.init(config)

    def save(self, config):
        self.pywallpaper.save(config)

    def createConfigurationInterface(self, parent):
        self.connect(self, SIGNAL('settingsChanged(bool)'), parent, SLOT('settingsChanged(bool)'))
        return self.pywallpaper.createConfigurationInterface(parent)

    def settingsChanged(self, changed):
        self.emit(SIGNAL('settingsChanged(bool)'), changed)

    def mouseMoveEvent(self, event):
        self.pywallpaper.mouseMoveEvent(event)

    def mousePressEvent(self, event):
        self.pywallpaper.mousePressEvent(event)

    def mouseReleaseEvent(self, event):
        self.pywallpaper.mouseReleaseEvent(event)

    def wheelEvent(self, event):
        self.pywallpaper.wheelEvent(event)

    @pyqtSignature("renderCompleted(const QImage&)")
    def renderCompleted(self, image):
        self.pywallpaper.renderCompleted(image)

    @pyqtSignature("urlDropped(const KUrl&)")
    def urlDropped(self, url):
        self.pywallpaper.urlDropped(url)

def CreatePlugin(widget_parent, parent, component_data):
    return PythonWallpaperScript(parent)
