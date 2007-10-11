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

#include "ion.h"
#include "ion.moc"

class IonInterface::Private : public QObject
{
public:
    Private(IonInterface *i)
            : ion(i) {}

    int ref;
    IonInterface *ion;
    bool valid;
    QVector<QString> ionSource;  // Array of this ions sources
};

IonInterface::IonInterface(QObject *parent)
        : Plasma::DataEngine(parent),
        d(new Private(this))
{
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

// Check if Ion is valid - Not used yet
bool IonInterface::isValid() const
{
    return d->valid;
}

// Set the Ions datasource name listing. Used by weather dataengine
void IonInterface::setSource(QString key)
{
    if (!d->ionSource.contains(key)) {
        d->ionSource.append(key);
    }
}

// Return an array of sources from a Ion
QVector<QString> IonInterface::ionSourceDict() const
{
    return d->ionSource;
}

// Deletes a datasource from the ion
void IonInterface::removeSource(QString key)
{
     QVector<QString>::iterator it;
     for (it = d->ionSource.begin(); it != d->ionSource.end(); ++it) {
         if (*it == key)  {
             kDebug() << "Going to remove: " << *it;
             d->ionSource.erase(it);
             break;
         }
     }
}
