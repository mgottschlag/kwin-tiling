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

class PythonAppletScript(Plasma.AppletScript):
    def __init__(self, parent):
        Plasma.AppletScript.__init__(self,parent)
        print("PythonAppletScript()")
        self.importer = plasma_importer.PlasmaImporter()
        self.initialized = False

    def init(self):
        print("Script Name: " + str(self.applet().name()))
        print("Script Category: " + str(self.applet().category()))

        self.applet().resize(200, 200)

        # applet() cannot be relied on to give the right details in the destructor,
        # so the plugin name is stored aside. (n.b module.__name__ cannot be relied
        # on either; it might have been changed in the module itself)
        self.m_moduleName = str(self.applet().pluginName())
        print("pluginname: " + str(self.applet().pluginName()))
        
        plugin_name = str(self.applet().pluginName()).replace('-','_')

        self.importer.register_top_level(plugin_name, str(self.applet().package().path()))

        print("mainScript: " + str(self.mainScript()))
        print("package path: " + str(self.applet().package().path()))

        # import the code at the file name reported by mainScript()
        self.module = __import__(plugin_name+'.main')
        self.pyapplet = self.module.main.CreateApplet(None)
        self.pyapplet.setApplet(self.applet())
        self.pyapplet.init()

        self._setUpEventHandlers()
        self.initialized = True
        return True

    def __dtor__(self):
        print("~PythonAppletScript()")
        self.pyapplet = None

    def constraintsEvent(self, constraints):
        if not self.initialized:
            return
        self.pyapplet.constraintsEvent(constraints)

    def showConfigurationInterface(self):
        if not self.initialized:
            return
        print("Script: showConfigurationInterface")
        self.pyapplet.showConfigurationInterface()

    def paintInterface(self, painter, option, contentsRect):
        if not self.initialized:
            return
        self.pyapplet.paintInterface(painter, option, contentsRect)

    def contextualActions(self):
        if not self.initialized:
            return

        print("pythonapplet contextualActions()")
        return self.pyapplet.contextualActions()

    def shape(self):
        if not self.initialized:
            return
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
