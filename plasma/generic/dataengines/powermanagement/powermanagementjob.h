/*
<<<<<<< HEAD
 *   Copyright 2011 Sebastian Kügler <sebas@kde.org>
=======
>>>>>>> 677b86151e254a7b0af614312d24a601a2a2e6cd
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

<<<<<<< HEAD
// plasma
=======
#include "powermanagementengine.h"

>>>>>>> 677b86151e254a7b0af614312d24a601a2a2e6cd
#include <Plasma/ServiceJob>

typedef QMap<QString, QString> StringStringMap;

class PowermanagementJob : public Plasma::ServiceJob
{
<<<<<<< HEAD

    Q_OBJECT

    public:
        PowerManagementJob(const QString &operation, QMap<QString, QVariant> &parameters,
                           QObject *parent = 0);
        ~PowerManagementJob();

    protected:
        void start();

    private:
        enum SuspendType { Ram, Disk, Hybrid };
        bool suspend(const SuspendType &type);
        void requestShutDown();
        QString callForType(const SuspendType &type);
=======
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
>>>>>>> 677b86151e254a7b0af614312d24a601a2a2e6cd
};

Q_DECLARE_METATYPE (StringStringMap);

#endif // POWERMANAGEMENT_JOB_H
