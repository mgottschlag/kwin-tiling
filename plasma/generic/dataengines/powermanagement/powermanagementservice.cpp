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

#include "powermanagementservice.h"
#include "powermanagementjob.h"
<<<<<<< HEAD

PowerManagementService::PowerManagementService(const QString &source)
    : m_id(source)
{
    setName("powermanagementservice");
}

ServiceJob *PowerManagementService::createJob(const QString &operation,
                                           QMap<QString, QVariant> &parameters)
{
    return new PowerManagementJob(operation, parameters, this);
=======
#include "powermanagementengine.h"

PowermanagementService::PowermanagementService (PowermanagementEngine* parent, const QString& source)
    : Plasma::Service(parent),
      m_engine (parent),
      m_dest (source)
{
    setName ("powermanagement");
    setDestination (source);
}

Plasma::ServiceJob* PowermanagementService::createJob (const QString& operation,
                                                       QMap<QString, QVariant>& parameters)
{
    return new PowermanagementJob (m_engine, m_dest, operation, parameters, this);
>>>>>>> 677b86151e254a7b0af614312d24a601a2a2e6cd
}

#include "powermanagementservice.moc"
