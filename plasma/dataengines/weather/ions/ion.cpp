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
            : ion(i),
              initialized(false)
    {}

    int ref;
    IonInterface *ion;
    bool valid;
    bool initialized;
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

bool IonInterface::sourceRequested(const QString &source)
{
    setData(source, Plasma::DataEngine::Data());
   
    if (d->initialized) {
        this->updateSource(source);
    }

    return true;
}

bool IonInterface::updateSource(const QString& source) 
{
     kDebug() << "SOURCE IS = " << source; 
     if (d->initialized) {
         if(this->updateIonSource(source)) {
            return true;
         } else {
            return false;
         }
     }

     return false; 
}

void IonInterface::setInitialized(bool initialized)
{
    d->initialized = initialized;

    if (d->initialized) {
        foreach (const QString &source, sources()) {
            updateSource(source);
        }
    }
}
