/***************************************************************************
 *   Copyright (C) 2007 by Shawn Starr <shawn.starr@rogers.com>            *
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

#ifndef _WEATHER_ENGINE_H_
#define _WEATHER_ENGINE_H_

#include <KService>
#include <KGenericFactory>
#include <plasma/dataengine.h>
#include "ions/ion.h"

class QTimer;

/* DataEngine class
   - Loads Ions (data sources from various inputs).
   - Handles interaction from Applet <-> Dataengine
*/
class WeatherEngine : protected Plasma::DataEngine
{
    Q_OBJECT

public:
    // ctor, dtor
    WeatherEngine(QObject *parent, const QVariantList &args);
    ~WeatherEngine();

    // Ion plugin methods
    IonInterface* Ion(const QString& name) const;  // Returns an Ion instance.
    IonInterface* loadIon(const QString& name);    // Loads an Ion plugin.
    void unloadIon(const QString& name);           // Unloads an Ion plugin.
    KService::List knownIons();                // Returns a list of Ion plugin names.

protected:
    // dataEngine method - We use it to communicate to the Ion plugins to set the data sources
    bool sourceRequested(const QString &);

protected slots:
    // SLOT: trigger to indicate new data is available from an Ion. There are two modes.
    // When using a timer no ion is specified, otherwise when loading an ion an ion is
    // specified.
    void updateData();
    void updated(const QString& source, Plasma::DataEngine::Data data);
    void newIonSource(const QString& source);
    void removeIonSource(const QString& source);

private:
    class Private;
    Private *const d;
};

K_EXPORT_PLASMA_DATAENGINE(weather, WeatherEngine)

#endif
