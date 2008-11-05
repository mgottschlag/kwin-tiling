#
# Copyright 2007 Aaron Seigo <aseigo@kde.org>
# Copyright 2008 Alex Merry <alex.merry@kdemail.net>
# Copyright 2008 Simon Edwards <simon@simonzone.com> (Translated to Python)
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
from PyKDE4.kdecore import *
from PyKDE4 import plasmascript

class PyTimeEngine(plasmascript.DataEngine):
    def __init__(self,parent,args=None):
        plasmascript.DataEngine.__init__(self,parent)

    def init(self):
        self.setMinimumPollingInterval(333)

        """
    QStringList TimeEngine::sources() const
{
    QStringList timezones(KSystemTimeZones::zones().keys());
    timezones << "Local";
    return timezones;
}
"""
    def sources(self):
        print(repr( KSystemTimeZones.zones() ))
        return ["Local"]
        #KSystemTimeZones.zones()

    def sourceRequestEvent(self, name):
        return self.updateSourceEvent(name)

    def updateSourceEvent(self, tz):
        localName = "Local"
        if tz == localName:
            self.setData(localName, "Time", QVariant(QTime.currentTime()))
            self.setData(localName, "Date", QVariant(QDate.currentDate()))
            # this is relatively cheap - KSTZ::local() is cached
            timezone = KSystemTimeZones.local().name()
        else:
            newTz = KSystemTimeZones.zone(tz)
            if not newTz.isValid():
                return False
            dt = KDateTime.currentDateTime(KDateTime.Spec(newTz))
            self.setData(tz, "Time", QVariant(dt.time()))
            self.setData(tz, "Date", QVariant(dt.date()))
            timezone = tz

        trTimezone = timezone
        self.setData(tz, "Timezone", QVariant(trTimezone));
        print("trTimezone:"+trTimezone)
        tzParts = str(trTimezone).split("/")
        if len(tzParts)>=2:
            self.setData(tz, "Timezone Continent", QVariant(tzParts[0]))
            self.setData(tz, "Timezone City", QVariant(tzParts[1]))

        return True

def CreateDataEngine(parent):
    return PyTimeEngine(parent)
