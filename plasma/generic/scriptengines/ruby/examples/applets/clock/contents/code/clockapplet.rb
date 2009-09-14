=begin
/***************************************************************************
 *   Copyright (C) 2007-2008 by Riccardo Iaconelli <riccardo@kde.org>      *
 *   Copyright (C) 2007-2008 by Sebastian Kuegler <sebas@kde.org>          *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/
=end

class ClockApplet < PlasmaScripting::Applet
  attr_accessor :currentTimezone

  slots :configAccepted, 
        'showCalendar(QGraphicsSceneMouseEvent *)',
        'currentTimezone=(QString)'

  def initialize(parent, args)
    super(parent, args)
    @calendar = nil
    @currentTimezone = "Local"

    @calendarUi = Ui::Calendar.new
    @timezonesUi = Ui::TimezonesConfig.new
    @clicked = Qt::Point.new
    @timeZones = []
  end

  def updateToolTipContent()
  end

  def createConfigurationInterface(parent)
    createClockConfigurationInterface(parent)

    widget = Qt::Widget.new
    @timezonesUi.setupUi(widget)

    parent.addPage(widget, KDE.i18n("Time Zones"), icon())

    @timezonesUi.localTimeZone.checked = localTimezone?
    @timezonesUi.timeZones.setSelected(@currentTimezone, true)
    @timezonesUi.timeZones.enabled = !localTimezone?

    parent.buttons = KDE::Dialog::Ok | KDE::Dialog::Cancel | KDE::Dialog::Apply
    connect(parent, SIGNAL(:applyClicked), self, SLOT(:configAccepted))
    connect(parent, SIGNAL(:okClicked), self, SLOT(:configAccepted))
  end

  def createClockConfigurationInterface(parent)
  end

  def clockConfigAccepted()
  end

  def configAccepted()
    cg = config()

    @timeZones = @timezonesUi.timeZones.selection()
    cg.writeEntry("timeZones", @timeZones)

    newTimezone = localTimezone()

    if !@timezonesUi.localTimeZone.checked? && !@timeZones.empty?
        newTimezone = @timeZones[0]
    end

    changeEngineTimezone(@currentTimezone, newTimezone)

    @currentTimezone = newTimezone
    cg.writeEntry("currentTimezone", newTimezone)

    clockConfigAccepted()

    constraintsEvent(Plasma::SizeConstraint)
    update()
    emit configNeedsSaving
  end

  def changeEngineTimezone(oldTimezone, newTimezone)
  end

  def mousePressEvent(event)
    if event.buttons == Qt::LeftButton
      @clicked = scenePos.toPoint
      event.accepted = true
        return
    end

    super(event)
  end

  def mouseReleaseEvent(event)
    if (@clicked - scenePos.toPoint).manhattanLength <
        KDE::GlobalSettings.dndEventDelay()
        showCalendar(event)
    end
  end

  def showCalendar(event)
    if @calendar == 0
      @calendar = Plasma::Dialog.new
      @calendarUi.setupUi(@calendar)
      @calendar.setWindowFlags(Qt::Popup)
      @calendar.adjustSize
    end

    if @calendar.visible?
      @calendar.hide
    else
      data = dataEngine("time").query(@currentTimezone)
      @calendarUi.kdatepicker.date = data["Date"].toDate
      @calendar.move(popupPosition(@calendar.sizeHint))
      @calendar.show
    end
  end

  def localTimezone?
    return @currentTimezone == localTimezone()
  end

  def localTimezone()
    return "Local"
  end
end
