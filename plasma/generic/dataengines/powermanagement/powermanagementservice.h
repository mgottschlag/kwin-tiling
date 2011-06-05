/*
 *   Copyright 2011 Sebastian Kügler <sebas@kde.org>
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

#ifndef POWERMANAGEMENTSERVICE_H
#define POWERMANAGEMENTSERVICE_H

#include <Plasma/Service>

class PowerManagementService : public Plasma::Service
{
    Q_OBJECT

public:
    PowerManagementService(const QString &source);
    ServiceJob *createJob(const QString &operation,
                          QMap<QString, QVariant> &parameters);

private:
    QString m_id;

};

#endif // POWERMANAGEMENTSERVICE_H
