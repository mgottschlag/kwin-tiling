
from PyQt4.QtCore import *
from PyQt4.QtGui import *
from PyKDE4.kdecore import *
from PyKDE4.kdeui import *
import plasma
from timezonesConfig_ui import *
from calendar_ui import *

class ClockApplet(plasma.Applet):
    def __init__(self,parent,args=None):
        plasma.Applet.__init__(self,parent)

        self.calendar = None
        self.currentTimezone = "Local"

        self.calendarUi = Ui_calendar()
        self.timezonesUi = Ui_timezonesConfig()
        self.clicked = QPoint()
        self.timeZones = []

    def updateToolTipContent(self):
        pass

    def createConfigurationInterface(self,parent):
        self.createClockConfigurationInterface(parent)

        #widget = QWidget(parent)
        #self.timezonesUi.setupUi(widget)

        #parent.addPage(widget, i18n("Time Zones"), self.icon())

        #self.timezonesUi.localTimeZone.checked = self.isLocalTimezone()
        #self.timezonesUi.timeZones.setSelected(self.currentTimezone, True)
        #self.timezonesUi.timeZones.setEnabled(not self.isLocalTimezone())

        parent.setButtons(KDialog.ButtonCodes(KDialog.ButtonCode(KDialog.Ok | KDialog.Cancel | KDialog.Apply)))
        self.connect(parent, SIGNAL("applyClicked"), self.configAccepted)
        self.connect(parent, SIGNAL("okClicked"), self.configAccepted)

    def createClockConfigurationInterface(self,parent):
        pass

    def clockConfigAccepted(self):
        pass

    def configAccepted(self):
        cg = self.config()

        self.timeZones = self.timezonesUi.timeZones.selection()
        cg.writeEntry("timeZones", self.timeZones)

        newTimezone = self.localTimezone()

        if not self.timezonesUi.localTimeZone.isChecked() and not self.timeZones.isEmpty():
            newTimezone = self.timeZones[0]

        self.changeEngineTimezone(self.currentTimezone, newTimezone)

        self.currentTimezone = newTimezone
        cg.writeEntry("currentTimezone", newTimezone)

        self.clockConfigAccepted()

        self.constraintsEvent(Plasma.SizeConstraint)
        self.update()
        self.emit(SIGNAL("configNeedsSaving()"))

    def changeEngineTimezone(self, oldTimezone, newTimezone):
        pass

    def mousePressEvent(self,event):
        if event.buttons() == Qt.LeftButton:
            self.clicked = self.scenePos.toPoint()
            event.setAccepted(True)
            return
        plasma.Applet.mousePressEvent(self,event)

    def mouseReleaseEvent(self,event):
        if (self.clicked - self.scenePos.toPoint()).manhattanLength() < \
                KGlobalSettings.dndEventDelay():
            self.showCalendar(event)

    def showCalendar(self,event):
        if self.calendar is None:
            self.calendar = Plasma.Dialog()
            self.calendarUi.setupUi(self.calendar)
            self.calendar.setWindowFlags(Qt.Popup)
            self.calendar.adjustSize()

        if self.calendar.isVisible():
            self.calendar.hide()
        else:
            data = self.dataEngine("time").query(self.currentTimezone)
            self.calendarUi.kdatepicker.date = data[QString("Date")].toDate()
            self.calendar.move(self.popupPosition(self.calendar.sizeHint()))
            self.calendar.show()

    def isLocalTimezone(self):
        return self.currentTimezone == self.localTimezone()

    def localTimezone(self):
        return "Local"
