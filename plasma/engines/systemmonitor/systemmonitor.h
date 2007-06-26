/*
 *   Copyright (C) 2007 John Tapsell <tapsell@kde.org>
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

#ifndef SYSTEMMONITORENGINE_H
#define SYSTEMMONITORENGINE_H

#include "plasma/dataengine.h"

#include "SensorClient.h"

#include <QStringList>

class QTimer;

/**
 * This class evaluates the basic expressions given in the interface.
 */
class SystemMonitorEngine : public Plasma::DataEngine, public KSGRD::SensorClient
{
    Q_OBJECT

    public:
	/** Inherited from Plasma::DataEngine.  Returns a list of all the sensors that ksysguardd knows about. */
        virtual QStringList sources() const;
        SystemMonitorEngine( QObject* parent, const QStringList& args );
        ~SystemMonitorEngine();

    protected:
        bool sourceRequested(const QString &name);
	/** inherited from SensorClient */
        virtual void answerReceived( const QString &sensor, const QList<QByteArray>&answer );
        virtual void sensorLost( const QString &sensor);


    protected slots:
        void updateSensors();

    private:
        QStringList m_sensors;
        QTimer* m_timer;
	int m_waitingFor;
};

K_EXPORT_PLASMA_DATAENGINE(systemmonitor, SystemMonitorEngine)

#endif

