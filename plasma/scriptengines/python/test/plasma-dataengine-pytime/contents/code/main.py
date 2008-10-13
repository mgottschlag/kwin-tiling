
from PyQt4.QtCore import *
import plasma

class TimeEngine(plasma.DataEngine):
    def __init__(self,parent,args=None):
        plasma.DataEngine.__init__(self,parent)
        self.setMinimumPollingInterval(333)
        """
    QStringList TimeEngine::sources() const
{
    QStringList timezones(KSystemTimeZones::zones().keys());
    timezones << "Local";
    return timezones;
}
"""
    def sourceRequestEvent(self, name):
        return self.updateSourceEvent(name)

    def updateSourceEvent(self, tz):
        localName = "Local"
        if tz == localName:
            self.setData(localName, "Time", QTime.currentTime())
            self.setData(localName, "Date", QDate.currentDate())
            # this is relatively cheap - KSTZ::local() is cached
            timezone = KSystemTimeZones.local().name()
        else:
            KTimeZone newTz = KSystemTimeZones.zone(tz)
            if not newTz.isValid():
                return False

            dt = KDateTime.currentDateTime(newTz)
            self.setData(tz, "Time", dt.time())
            self.setData(tz, "Date", dt.date())
            timezone = tz

        trTimezone = timezone
        self.setData(tz, "Timezone", trTimezone);
        tzParts = str(trTimezone).split("/")

        self.setData(tz, "Timezone Continent", tzParts[0])
        self.setData(tz, "Timezone City", tzParts[1])

        return True
