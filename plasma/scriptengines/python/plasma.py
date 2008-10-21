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

# Plasma applet API for Python

from PyKDE4.plasma import Plasma # Plasma C++ namespace
from PyQt4.QtCore import QObject
import sip
import gc

class Applet(QObject):
    ''' Subclass Applet in your module and return an instance of it in a global function named 
    applet(). Implement the following functions to breathe life into your applet:
        * paint - Draw the applet given a QPainter and some options
    It provides the same API as Plasma.Applet; it just has slightly less irritating event names. '''
    def __init__(self, parent=None):
        # this should be set when the applet is created
        QObject.__init__(self, parent)
        #sip.settracemask(0x3f)
        self.applet = None
        self._forward_to_applet = True

    def __getattr__(self, key):
        # provide transparent access to the real applet instance
        if self._forward_to_applet:
            return getattr(self.applet, key)
        else:
            raise AttributeError(key)
    
    def _enableForwardToApplet(self):
        self._forward_to_applet = True
    def _disableForwardToApplet(self):
        self._forward_to_applet = False

    # Events

#    def _long_setApplet(self,applet):
#        self.setApplet(sip.wrapinstance(applet,Plasma.Applet))

    def setApplet(self,applet):
        self.applet = applet

    def init(self):
        pass
    
    def paintInterface(self, painter, options, rect):
        pass
        
    def constraintsEvent(self, flags):
        pass
        
#    def _long_constraintsEvent(self, flags):
#        self.constraintsEvent(Plasma.Constraints(Plasma.Constraint(flags)))

    def showConfigurationInterface(self):
        pass

    def contextualActions(self):
        return []

    def setHasConfigurationInterface(self,hasInterface):
        Plasma.AppletProtectedThunk.static_public_setHasConfigurationInterface(self.applet,hasInterface)

###########################################################################
class DataEngine(QObject):
    def __init__(self, parent=None):
        QObject.__init__(self, parent)
        self.dataengine = None

    def setDataEngineScript(self,dataEngineScript):
        self.data_engine_script = dataEngineScript

    def __getattr__(self, key):
        # provide transparent access to the real dataengine instance
        #if self._forward_to_applet:
        return getattr(self.data_engine_script, key)
        #else:
        #    raise AttributeError(key)

    def init(self):
        pass

    def sourceRequestEvent(self,name):
        return False

    def updateSourceEvent(self,source):
        return False
