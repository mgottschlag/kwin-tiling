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

<<<<<<< HEAD
#ifndef POWERMANAGEMENTSERVICE_H
#define POWERMANAGEMENTSERVICE_H

#include <Plasma/Service>

class PowerManagementService : public Plasma::Service
=======
#ifndef POWERMANAGEMENT_SERVICE_H
#define POWERMANAGEMENT_SERVICE_H

#include <Plasma/Service>

class PowermanagementEngine;

class PowermanagementService : public Plasma::Service
>>>>>>> 677b86151e254a7b0af614312d24a601a2a2e6cd
{
    Q_OBJECT

public:
<<<<<<< HEAD
    PowerManagementService(const QString &source);
    ServiceJob *createJob(const QString &operation,
                          QMap<QString, QVariant> &parameters);

private:
    QString m_id;

};

#endif // POWERMANAGEMENTSERVICE_H
=======
    PowermanagementService(PowermanagementEngine* parent, const QString& source);

protected:
    Plasma::ServiceJob* createJob(const QString& operation,
                  QMap<QString, QVariant>& parameters);

private:
    PowermanagementEngine* m_engine;
    QString m_dest;
};

#endif // POWERMANAGEMENT_SERVICE_H

>>>>>>> 677b86151e254a7b0af614312d24a601a2a2e6cd
