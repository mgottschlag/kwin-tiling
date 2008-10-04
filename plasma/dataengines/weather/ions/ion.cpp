/***************************************************************************
 *   Copyright (C) 2007-2008 by Shawn Starr <shawn.starr@rogers.com>       *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA          *
 ***************************************************************************/

#include "ion.h"
#include "ion.moc"

class IonInterface::Private : public QObject
{
public:
    Private(IonInterface *i)
            : ion(i),
            initialized(false) {}

    int ref;
    IonInterface *ion;
    bool initialized;
};

IonInterface::IonInterface(QObject *parent, const QVariantList &args)
        : Plasma::DataEngine(parent, args),
        d(new Private(this))
{
// Initialize the loaded ion with a reference count of 0.
    d->ref = 0;
}

// Increment reference counter
void IonInterface::ref()
{
    ++d->ref;
}

// Decrement reference counter
void IonInterface::deref()
{
    --d->ref;
}

// Check if Ion is used
bool IonInterface::isUsed() const
{
    return d->ref != 0;
}

/**
 * If the ion is not initialized just set the initial data source up even if it's empty, we'll retry once the initialization is done
 */
bool IonInterface::sourceRequestEvent(const QString &source)
{
    kDebug() << "sourceRequested()";
    if (d->initialized) {
        return updateIonSource(source);
    } else {
        setData(source, Plasma::DataEngine::Data());
    }

    return true;
}

/**
 * Update the ion's datasource. Triggered when a Plasma::DataEngine::connectSource() timeout occurs.
 */
bool IonInterface::updateSourceEvent(const QString& source)
{
    kDebug() << "updateSource()";
    if (d->initialized) {
        return updateIonSource(source);
    }

    return false;
}

/**
 * Set the ion to make sure it is ready to get real data.
 */
void IonInterface::setInitialized(bool initialized)
{
    d->initialized = initialized;

    if (d->initialized) {
        foreach(const QString &source, sources()) {
            updateSourceEvent(source);
        }
    }
}

/**
 * Return weather icon to display in an applet when given a condition.
 */
QString IonInterface::getWeatherIcon(const QMap<QString, ConditionIcons> &ConditionList, const QString& condition)
{
    switch (ConditionList[condition.toLower()]) {
    case ClearDay:
        return "weather-clear";
    case FewCloudsDay:
        return "weather-few-clouds";
    case PartlyCloudyDay:
        return "weather-clouds";
    case Overcast:
        return "weather-many-clouds";
    case Rain:
        return "weather-showers";
    case LightRain:
        return "weather-showers-scattered";
    case Showers:
        return "weather-showers-scattered";
    case ChanceShowersDay:
        return "weather-showers-scattered-day";
    case ChanceShowersNight:
        return "weather-showers-scattered-night";
    case Thunderstorm:
        return "weather-storm";
    case Hail:
        return "weather-hail";
    case Snow:
        return "weather-snow";
    case LightSnow:
        return "weather-snow-scattered";
    case Flurries:
        return "weather-snow-scattered";
    case FewCloudsNight:
        return "weather-few-clouds-night";
    case PartlyCloudyNight:
        return "weather-clouds-night";
    case ClearNight:
        return "weather-clear-night";
    case Mist:
        return "weather-mist";
    case NotAvailable:
        return "weather-none-available";
    }
    return "weather-none-available";
}
