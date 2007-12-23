/*
 *   Copyright (C) 2007 Aaron Seigo <aseigo@kde.org>
 *   Copyright (C) 2007 Sebastian Kuegler <sebas@kde.org>
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

#ifndef POWERMANAGEMENTENGINE_H
#define POWERMANAGEMENTENGINE_H

#include "plasma/dataengine.h"

#include <solid/battery.h>
#include <solid/acadapter.h>

#include <QHash>

/**
 * This class provides runtime information about the battery and AC status
 * for use in a simple batter monitor application.
 */
class PowermanagementEngine : public Plasma::DataEngine
{
    Q_OBJECT

public:
    PowermanagementEngine( QObject* parent, const QVariantList& args );
    ~PowermanagementEngine();
    QStringList sources() const;

protected:
    bool sourceRequested(const QString &name);
    void init();

protected slots:
    void updateBatteryChargeState(int newState, const QString& udi);
    void updateBatteryPlugState(bool newState, const QString& udi);
    void updateBatteryChargePercent(int newValue, const QString& udi);
    void updateAcPlugState(bool newState);

private:
    Solid::AcAdapter* m_acadapter;
    QStringList m_sources;
    
    QHash<QString, QString> m_batterySources;
    
};

K_EXPORT_PLASMA_DATAENGINE(powermanagement, PowermanagementEngine)

#endif
