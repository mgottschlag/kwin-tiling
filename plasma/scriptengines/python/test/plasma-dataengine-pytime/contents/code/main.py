
from PyQt4.QtCore import *
from PyKDE4.kdecore import *
import plasma

class PyTimeEngine(plasma.DataEngine):
    def __init__(self,parent,args=None):
        plasma.DataEngine.__init__(self,parent)

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