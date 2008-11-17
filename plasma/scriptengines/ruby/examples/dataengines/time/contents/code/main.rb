=begin
/*
 *   Copyright 2007 Aaron Seigo <aseigo@kde.org>
 *
 *   Translated to Ruby by Richard Dale
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2 or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */
=end

require 'plasma_applet'

module RubyTime

class Main < PlasmaScripting::DataEngine

  def initialize(parent, args = nil)
    super(parent)
  end

  def init
    setMinimumPollingInterval(333)

    # To have translated timezone names
    # (effectively a noop if the catalog is already present).
    KDE::Global.locale.insertCatalog("timezones4")
  end

  def sources
    timezones = KDE::SystemTimeZones.zones.keys
    timezones << "Local"
    return timezones
  end

  def sourceRequestEvent(name)
    return updateSourceEvent(name)
  end

  def updateSourceEvent(tz)
    # puts "TimeEngine#updateTime"
    localName = I18N_NOOP("Local")
    if tz == localName
        setData(localName, I18N_NOOP("Time"), Qt::Time.currentTime)
        setData(localName, I18N_NOOP("Date"), Qt::Date.currentDate)
        # this is relatively cheap - KSTZ.local is cached
        timezone = KDE::SystemTimeZones.local.name
    else
        newTz = KDE::SystemTimeZones.zone(tz)
        unless newTz.valid?
            return false
        end

        dt = KDE::DateTime.currentDateTime(KDE::DateTime::Spec.new(newTz))
        setData(tz, I18N_NOOP("Time"), dt.time)
        setData(tz, I18N_NOOP("Date"), dt.date)
        timezone = tz
    end

    trTimezone = i18n(timezone)
    setData(tz, I18N_NOOP("Timezone"), trTimezone)
    tzParts = trTimezone.split("/")

    setData(tz, I18N_NOOP("Timezone Continent"), tzParts[0])
    setData(tz, I18N_NOOP("Timezone City"), tzParts[1])

    return true
  end
end

end
