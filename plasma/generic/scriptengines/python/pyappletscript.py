# -*- coding: utf-8 -*-
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
import os.path

class PythonAppletScript(Plasma.AppletScript):
    importer = None

    def __init__(self, parent):
        Plasma.AppletScript.__init__(self,parent)
        #print("PythonAppletScript()")
        if PythonAppletScript.importer is None:
             PythonAppletScript.importer = plasma_importer.PlasmaImporter()
        self.initialized = False

    def init(self):
        #print("Script Name: " + str(self.applet().name()))
        #print("Script Category: " + str(self.applet().category()))

        # applet() cannot be relied on to give the right details in the destructor,
        # so the plugin name is stored aside. (n.b module.__name__ cannot be relied
        # on either; it might have been changed in the module itself)
        self.moduleName = str(self.applet().pluginName())
        #print("pluginname: " + str(self.applet().pluginName()))
        self.pluginName = 'applet_' + self.moduleName.replace('-','_')

        PythonAppletScript.importer.register_top_level(self.pluginName, str(self.applet().package().path()))

        #print("mainScript: " + str(self.mainScript()))
        #print("package path: " + str(self.applet().package().path()))

        # import the code at the file name reported by mainScript()
        relpath = os.path.relpath(str(self.mainScript()),str(self.applet().package().path()))
        if relpath.startswith("contents/code/"):
            relpath = relpath[14:]
        if relpath.endswith(".py"):
            relpath = relpath[:-3]
        relpath = relpath.replace("/",".")
        self.module = __import__(self.pluginName+'.'+relpath)

        # Plasma main scripts not necessarily are named "main"
        # So we use __dict__ to get the right main script name
        basename = os.path.basename(str(self.mainScript()))
        basename = os.path.splitext(basename)[0]
        self.pyapplet = self.module.__dict__[basename].CreateApplet(None)
        self.pyapplet.setApplet(self.applet())
        self.pyapplet.setAppletScript(self)
        self.connect(self.applet(), SIGNAL('extenderItemRestored(Plasma::ExtenderItem*)'),
                     self, SLOT('initExtenderItem(Plasma::ExtenderItem*)'))
        self.connect(self, SIGNAL('saveState(KConfigGroup&)'),
                     self, SLOT('saveStateSlot(KConfigGroup&)'))
        self._setUpEventHandlers()
        self.initialized = True

        self.pyapplet.init()
        return True

    def __dtor__(self):
        #print("~PythonAppletScript()")
        if not self.initialized:
            return

        PythonAppletScript.importer.unregister_top_level(self.pluginName)
        self.pyapplet = None

    @pyqtSignature("initExtenderItem(Plasma::ExtenderItem*)")
    def initExtenderItem(self, item):
        if not self.initialized:
            return
        self.pyapplet.initExtenderItem(item)

    @pyqtSignature("saveStateSlot(KConfigGroup&)")
    def saveStateSlot(self, config):
        if not self.initialized:
            return
        self.pyapplet.saveState(config)

    def constraintsEvent(self, constraints):
        if not self.initialized:
            return
        self.pyapplet.constraintsEvent(constraints)

    def showConfigurationInterface(self):
        if not self.initialized:
            return
        #print("Script: showConfigurationInterface")
        self.pyapplet.showConfigurationInterface()

    def configChanged(self):
        if not self.initialized:
            return
        self.pyapplet.configChanged()

    def paintInterface(self, painter, option, contentsRect):
        if not self.initialized:
            return
        self.pyapplet.paintInterface(painter, option, contentsRect)

    def contextualActions(self):
        if not self.initialized:
            return

        #print("pythonapplet contextualActions()")
        return self.pyapplet.contextualActions()

    def shape(self):
        if not self.initialized:
            return QPainterPath()
        return self.pyapplet.shape()

    def eventFilter(self, obj, event):
        handler = self.event_handlers.get(event.type(),None)
        if handler is not None:
            apply(getattr(self.pyapplet,handler), (event,) )
            return True
        else:
            return False

    def _setUpEventHandlers(self):
        self.event_handlers = {}

        self.pyapplet._disableForwardToApplet()
        for event_type,handler in {
            QEvent.GraphicsSceneMousePress: "mousePressEvent",
            QEvent.GraphicsSceneContextMenu: "contextMenuEvent",
            QEvent.GraphicsSceneDragEnter: "dragEnterEvent",
            QEvent.GraphicsSceneDragLeave: "dragLeaveEvent",
            QEvent.GraphicsSceneDragMove: "dragMoveEvent",
            QEvent.GraphicsSceneDrop: "dropEvent",
            QEvent.FocusIn: "focusInEvent",
            QEvent.FocusOut: "focusOutEvent",
            QEvent.GraphicsSceneHoverEnter: "hoverEnterEvent",
            QEvent.GraphicsSceneHoverLeave: "hoverLeaveEvent",
            QEvent.GraphicsSceneHoverMove: "hoverMoveEvent",
            QEvent.InputMethod: "inputMethodEvent",
            QEvent.KeyPress: "keyPressEvent",
            QEvent.KeyRelease: "keyReleaseEvent",
            QEvent.GraphicsSceneMouseDoubleClick: "mouseDoubleClickEvent",
            QEvent.GraphicsSceneMouseMove: "mouseMoveEvent",
            QEvent.GraphicsSceneMousePress: "mousePressEvent",
            QEvent.GraphicsSceneMouseRelease: "mouseReleaseEvent",
            QEvent.GraphicsSceneWheel: "wheelEvent"
        }.iteritems():
            if hasattr(self.pyapplet,handler):
                self.event_handlers[event_type] = handler
        self.pyapplet._enableForwardToApplet()

        if self.event_handlers:
            self.applet().installEventFilter(self)

def CreatePlugin(widget_parent, parent, component_data):
    return PythonAppletScript(parent)
