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
#include <Plasma/DataEngine>

// Interface to handle a Ion plugin
class KDE_EXPORT IonInterface : public Plasma::DataEngine
{
    Q_OBJECT
public:
    typedef QHash<QString, IonInterface*> IonDict; // Define Dict as a QHash for Ions

    // ctor, dtor
    IonInterface(QObject *parent = 0);
    virtual ~IonInterface() {}

    // Pure virtuals, not instantianted
    virtual void init(void) = 0;   // Ion setup - Implemented by Ion plugin
    virtual void fetch(void) = 0;  // Get the Ion's data - Implemented by Ion plugin

    virtual bool validLocation(QString) = 0; // Check if city is valid otherwise returns false.

    void ref();   // Increment Ion counter
    void deref(); // Decrement Ion counter

    bool isUsed() const;  // Checks if Ion is used or not
    bool isValid() const; // Checks if Ion is valid -- Not used yet

    virtual void updateData(void) = 0; // Update the Ions weather information

    QVector<QString> ionSourceDict() const; // Returns array of sources an ion has
    void setSource(QString key);            // Sets a dataSource name for an ion

    enum ionOptions { UNITS, TIMEFORMAT };
    virtual void setIonOption(int option, QVariant option) = 0; // Sets an ion option.

private:
    class Private;
    Private* const d;
};

#define K_EXPORT_PLASMA_ION(name, classname) \
    K_EXPORT_COMPONENT_FACTORY(          \
                                         ion_##name,          \
                                         KGenericFactory<classname>("ion_" #name))
#endif
