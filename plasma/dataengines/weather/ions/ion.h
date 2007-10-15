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

#ifndef _ION_H
#define _ION_H

#include <QObject>
#include <kdemacros.h>
#include <KGenericFactory>
#include <plasma/dataengine.h>

/**
* This is the base class to be used to implement new ions for the WeatherEngine.
* The idea is that you can have multiple ions which provide weather information from different services to the engine from which an applet will request the data from.
* 
* Basically an ion is a Plasma::DataEngine, which is queried by the WeatherEngine instead of some applet.
*/
class KDE_EXPORT IonInterface : public Plasma::DataEngine
{
    Q_OBJECT
public:
    typedef QHash<QString, IonInterface*> IonDict; // Define Dict as a QHash for Ions

    /**
    * Constructor for the ion
    */
    IonInterface(QObject *parent = 0);
    /**
    * Destructor for the ion
    */
    virtual ~IonInterface() {}

    /**
    * Reimplement to do the initialization of the ion.
    * For example fetching the list of available cities or weather data sources should be fetched here.
    */
    virtual void init(void) = 0;

    /**
    * Increment ion counter. This is used to watch if the ion is being used.
    */
    void ref();

    /**
    * Decrement ion counter.
    */
    void deref();

    /**
    * Returns whether the ion is being used.
    * @return true if the ion is being used, false otherwise
    */
    bool isUsed() const;

    /**
    * Returns whether the ion is valid. Not used for now.
    * @return true if the ion is valid.
    */
    bool isValid() const;

    enum ionOptions { UNITS, TIMEFORMAT, WINDFORMAT };
    /**
    * Reimplement to set the wanted options for the ion such as unit and time, and wind speed formats.
    */
    virtual void option(int option, QVariant value) = 0;

public slots:
    bool updateSource(const QString& source);

protected:
    /**
     * Call this method to flush waiting source requests that may be pending
     * initialization
     *
     * @arg initialized whether or not the ion is currently ready to fetch data
     */
    void setInitialized(const bool initialized);

    /**
     * reimplemented from DataEngine
     */
    bool sourceRequested(const QString &name);

    /**
     * Reimplement to fetch the data from the ion
     * 
     */
    virtual bool updateIonSource(const QString &name) = 0;
   
private:
    class Private;
    Private* const d;
};

#define K_EXPORT_PLASMA_ION(name, classname) \
K_PLUGIN_FACTORY(factory, registerPlugin<classname>();) \
K_EXPORT_PLUGIN(factory("ion_" #name))
#endif
