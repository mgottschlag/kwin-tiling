/*
 *   Copyright (C) 2007 Matt Broadstone <mbroadst@gmail.com>
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

#ifndef DEVICESENGINE_H
#define DEVICESENGINE_H

#include <plasma/dataengine.h>

/**
 * This class provides runtime information about the battery and AC status
 * for use in a simple batter monitor application.
 */
class DevicesEngine : public Plasma::DataEngine
{
    Q_OBJECT
public:
    DevicesEngine(QObject *parent, const QStringList &args);
    ~DevicesEngine();

    QStringList sources() const;

protected:
    bool sourceRequested(const QString &name);

protected slots:

private:
    QStringList m_sources;

    QList<Solid::Device*> m_hardDrives;

};

K_EXPORT_PLASMA_DATAENGINE(devices, DevicesEngine)

#endif
