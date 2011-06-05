/*
 *   Copyright 2011 Viranch Mehta <viranch.mehta@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2 as
 *   published by the Free Software Foundation
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

#ifndef POWERMANAGEMENT_JOB_H
#define POWERMANAGEMENT_JOB_H

#include "powermanagementengine.h"

#include <Plasma/ServiceJob>

typedef QMap<QString, QString> StringStringMap;

class PowermanagementJob : public Plasma::ServiceJob
{
    Q_OBJECT
    
public:
    PowermanagementJob (PowermanagementEngine* engine,
                        const QString& destination,
                        const QString& operation,
                        QMap<QString,QVariant>& parameters,
                        QObject* parent = 0)
    : ServiceJob (destination, operation, parameters, parent),
      m_engine(engine)
    {
    }
    
    void start();
    
private:
    PowermanagementEngine* m_engine;
};

Q_DECLARE_METATYPE (StringStringMap);

#endif // POWERMANAGEMENT_JOB_H
