#
# Copyright (C) 2005,2006,2007 by Siraj Razick
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
from PyQt4.QtGui import *
from PyKDE4.kdecore import *
from PyKDE4.kdeui import *
from PyKDE4.plasma import Plasma
from analog_clock_config_ui import *
from calendar_ui import *
from PyKDE4 import plasmascript

class AnalogClockConfig(QWidget,Ui_clockConfig):
    def __init__(self,parent):
        QWidget.__init__(self,parent)
        self.setupUi(self)
        self.connect(self.localTimeZone, SIGNAL("stateChanged(int)"), self, SLOT("slotLocalTimeZoneToggled(int)"))

    @pyqtSignature("slotLocalTimeZoneToggled(int)")    
    def slotLocalTimeZoneToggled(self,b):
        self.timeZones.setDisabled(b)

class PyClockApplet(plasmascript.Applet):
    def __init__(self,parent,args=None):
        plasmascript.Applet.__init__(self,parent)

        self.calendar = None
        self.currentTimezone = "Local"
        self.clicked = QPoint()
        self.timeZones = []

    def init(self):
        KGlobal.locale().insertCatalog("libplasmaclock")

        self.setHasConfigurationInterface(True)
        self.resize(125, 125)
        self.setAspectRatioMode(Plasma.Square)

        self.timezone = ""
        self.showTimeString = False
        self.showSecondHand = False

        self.dialog = None
        self.calendarUi = Ui_calendar()
        self.lastTimeSeen = QTime()
        self.time = QTime()

        self.theme = Plasma.Svg(self)
        self.theme.setImagePath("widgets/clock")    # FIXME pull this out of this applet itself.
        self.theme.setContainsMultipleImages(False)
        self.theme.resize(self.size())

        cg = self.config()
        self.showTimeString = cg.readEntry("showTimeString", QVariant(False)).toBool()
        self.showSecondHand = cg.readEntry("showSecondHand", QVariant(False)).toBool()
        self.fancyHands = cg.readEntry("fancyHands", QVariant(False)).toBool()
        self.currentTimezone = cg.readEntry("timezone", self.localTimezone())

        self.connectToEngine()

    def connectToEngine(self):
        self.timeEngine = self.dataEngine("time")
        if self.showSecondHand:
            self.timeEngine.connectSource(self.currentTimezone, self, 500)
        else:
            self.timeEngine.connectSource(self.currentTimezone, self, 6000, Plasma.AlignToMinute)

    def constraintsEvent(self, constraints):
        if constraints & Plasma.FormFactorConstraint:
            self.setBackgroundHints(Plasma.Applet.NoBackground)
        if constraints & Plasma.SizeConstraint:
            self.theme.resize(self.size())

    def shape(self):
        if self.theme.hasElement("hint-square-clock"):
            return plasma.Applet.shape(self)

        path = QPainterPath()
        path.addEllipse(self.boundingRect().adjusted(-2, -2, 2, 2))
        return path

    @pyqtSignature("dataUpdated(const QString &, const Plasma::DataEngine::Data &)")
    def dataUpdated(self, sourceName, data):
        self.time = data[QString("Time")].toTime()

        if self.time.minute() == self.lastTimeSeen.minute() and \
                self.time.second() == self.lastTimeSeen.second():
            # avoid unnecessary repaints
            return

        self.lastTimeSeen = self.time
        self.update()

    def updateToolTipContent(self):
        pass

    def mousePressEvent(self,event):
        if event.buttons() == Qt.LeftButton:
            self.clicked = self.scenePos().toPoint()
            event.setAccepted(True)

    def mouseReleaseEvent(self,event):
        if (self.clicked - self.scenePos().toPoint()).manhattanLength() < \
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

    def showConfigurationInterface(self):
        windowTitle = str(self.applet.name()) + " Settings" #i18nc("@title:window", "%s Settings" % str(self.applet.name()))

        if self.dialog is None:
            self.dialog = KDialog(None)
            self.dialog.setWindowTitle(windowTitle)

            self.ui = AnalogClockConfig(self.dialog)
            self.dialog.setMainWidget(self.ui)

            self.dialog.setButtons(KDialog.ButtonCodes(KDialog.ButtonCode(KDialog.Ok | KDialog.Cancel | KDialog.Apply)))
            self.dialog.showButton(KDialog.Apply, False)

            self.connect(self.dialog, SIGNAL("applyClicked()"), self, SLOT("configAccepted()"))
            self.connect(self.dialog, SIGNAL("okClicked()"), self, SLOT("configAccepted()"))
            #self.connect(self.dialog, SIGNAL("applyClicked()"), self.configAccepted)
            #self.connect(self.dialog, SIGNAL("okClicked()"), self.configAccepted)

        self.ui.showTimeStringCheckBox.setChecked(self.showTimeString)
        self.ui.showSecondHandCheckBox.setChecked(self.showSecondHand)
        self.ui.localTimeZone.setChecked(self.isLocalTimezone())
        self.ui.timeZones.setSelected(self.currentTimezone, True)
        self.ui.timeZones.setEnabled(not self.isLocalTimezone())

        self.dialog.show()

    @pyqtSignature("configAccepted()")    
    def configAccepted(self):
        cg = self.config()

        # Timezones
        self.timeZones = self.ui.timeZones.selection()
        cg.writeEntry("timeZones", self.timeZones)
        newTimezone = self.localTimezone()
        if not self.ui.localTimeZone.isChecked() and not self.timeZones.isEmpty():
            newTimezone = self.timeZones[0]
        self.changeEngineTimezone(self.currentTimezone, newTimezone)
        self.currentTimezone = newTimezone
        cg.writeEntry("currentTimezone", newTimezone)

        # Display
        self.showTimeString = self.ui.showTimeStringCheckBox.isChecked()
        self.showSecondHand = self.ui.showSecondHandCheckBox.isChecked()
        cg.writeEntry("showTimeString", QVariant(self.showTimeString))
        cg.writeEntry("showSecondHand", QVariant(self.showSecondHand))

        self.dataEngine("time").disconnectSource(self.currentTimezone, self)
        self.connectToEngine()

        self.constraintsEvent(Plasma.SizeConstraint)
        self.update()
        self.emit(SIGNAL("configNeedsSaving()"))

    def changeEngineTimezone(self, oldTimezone, newTimezone):
        self.dataEngine("time").disconnectSource(oldTimezone, self)
        self.timeEngine = self.dataEngine("time")
        if self.showSecondHand:
            self.timeEngine.connectSource(newTimezone, self, 500)
        else:
            self.timeEngine.connectSource(newTimezone, self, 6000, Plasma.AlignToMinute)

    def drawHand(self, painter, rotation, handName):
        painter.save()
        boundSize = self.boundingRect().size()
        elementRect = self.theme.elementRect(handName)

        painter.translate(boundSize.width() / 2, boundSize.height() / 2)
        painter.rotate(rotation)
        painter.translate(-elementRect.width() / 2,
            -(self.theme.elementRect("clockFace").center().y() - elementRect.top()))
        self.theme.paint(painter, QRectF(QPointF(0.0, 0.0), elementRect.size()), handName)

        painter.restore()

    def paintInterface(self, painter, option, rect):
        tempRect = QRectF(0, 0, 0, 0)

        boundSize = self.size()

        painter.setRenderHint(QPainter.SmoothPixmapTransform)

        minutes = 6.0 * self.time.minute() - 180
        hours = 30.0 * self.time.hour() - 180 + ((self.time.minute() / 59.0) * 30.0)

        self.theme.paint(painter, QRectF(rect), "ClockFace")

        if self.showTimeString:
            fm = QFontMetrics(QApplication.font())
            margin = 4
            if self.showSecondHand:
                # FIXME: temporary time output
                time = self.time.toString()
            else:
                time = self.time.toString("hh:mm")

            textRect = QRect((rect.width()/2 - fm.width(time) / 2),((rect.height() / 2) - fm.xHeight() * 4), fm.width(time), fm.xHeight())

            painter.pen = Qt.NoPen
            background = Plasma.Theme.defaultTheme().color(Plasma.Theme.BackgroundColor)
            background.setAlphaF(0.5)
            painter.brush = QBrush(background)

            painter.setRenderHint(QPainter.Antialiasing, True)
            painter.drawPath(Plasma.PaintUtils.roundedRectangle(QRectF(textRect.adjusted(-margin, -margin, margin, margin)), margin))
            painter.setRenderHint(QPainter.Antialiasing, False)

            painter.pen = Plasma.Theme.defaultTheme().color(Plasma.Theme.TextColor)
              
            painter.drawText(textRect.bottomLeft(), time)

        # Make sure we paint the second hand on top of the others
        if self.showSecondHand:
            anglePerSec = 6.0
            seconds = anglePerSec * self.time.second() - 180

        if self.theme.hasElement("HourHandShadow"):
            painter.translate(1,3)
            self.drawHand(painter, hours, "HourHandShadow")
            self.drawHand(painter, minutes, "MinuteHandShadow")

            if self.showSecondHand:
                self.drawHand(painter, seconds, "SecondHandShadow")

            painter.translate(-1,-3)
        
        self.drawHand(painter, hours, "HourHand")
        self.drawHand(painter, minutes, "MinuteHand")
        if self.showSecondHand:
            self.drawHand(painter, seconds, "SecondHand")

        painter.save()
        self.theme.resize(boundSize)
        elementSize = QSizeF(self.theme.elementSize("HandCenterScrew"))
        tempRect.size = elementSize
        painter.translate(boundSize.width() / 2.0 - elementSize.width() / 2.0, boundSize.height() / 2.0 - elementSize.height() / 2.0)
        self.theme.paint(painter, tempRect, "HandCenterScrew")
        painter.restore()

        self.theme.paint(painter, QRectF(rect), "Glass")

def CreateApplet(parent):
    return PyClockApplet(parent)
