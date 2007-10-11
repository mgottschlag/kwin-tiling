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
    * Reimplement to do the fetching of weather data.
    * This is being called just before calling updateData() 
    */
    virtual void fetch(void) = 0;

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

    /**
    * Reimplement this to set the data to the engine by using setData() calls.
    * You may have to force to call this method inside your ion everytime new data has been arrived from the fetching job.
    */
    virtual void updateData(void) = 0;

    /**
    * Returns the source array.
    * @return Returns the array of source the ion has.
    */
    QVector<QString> ionSourceDict() const;
    /**
    * Set the datasource name to the array of sources
    * @param key The name of the datasource
    */
    void setSource(QString key);

    /**
    * Remove a datasource entry from the array
    * @param key The name of the datasource
    */
    void removeSource(QString key);

    enum ionOptions { UNITS, TIMEFORMAT, WINDFORMAT };
    /**
    * Reimplement to set the wanted options for the ion such as unit and time, and wind speed formats.
    */
    virtual void option(int option, QVariant value) = 0;

private:
    class Private;
    Private* const d;
};

#define K_EXPORT_PLASMA_ION(name, classname) \
K_PLUGIN_FACTORY(factory, registerPlugin<classname>();) \
K_EXPORT_PLUGIN(factory("ion_" #name))
#endif
