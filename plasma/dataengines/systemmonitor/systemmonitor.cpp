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

#include "systemmonitor.h"

#include <QTimer>
#include <QProcess>

#include <KLocale>

#include <Plasma/DataContainer>

#include "../../ksysguard/gui/ksgrd/SensorManager.h"

SystemMonitorEngine::SystemMonitorEngine(QObject* parent, const QVariantList& args)
    : Plasma::DataEngine(parent)
{
    Q_UNUSED(args)

    KSGRD::SensorMgr = new KSGRD::SensorManager(this);
    KSGRD::SensorMgr->engage("localhost", "", "ksysguardd");

    m_waitingFor= 0;
    KSGRD::SensorMgr->sendRequest("localhost", "monitors", (KSGRD::SensorClient*)this, -1);
}

SystemMonitorEngine::~SystemMonitorEngine()
{
}

QStringList SystemMonitorEngine::sources() const
{
    return m_sensors; 
}

bool SystemMonitorEngine::sourceRequestEvent(const QString &name)
{
    Q_UNUSED(name);
    return false;
}
bool SystemMonitorEngine::updateSourceEvent(const QString &sensorName)
{
    KSGRD::SensorMgr->sendRequest( "localhost", sensorName, (KSGRD::SensorClient*)this, m_sensors.indexOf(sensorName));
    KSGRD::SensorMgr->sendRequest( "localhost", QString("%1?").arg(sensorName), (KSGRD::SensorClient*)this, -(m_sensors.indexOf(sensorName)+2));
    return false;
}

void SystemMonitorEngine::updateSensors()
{
    DataEngine::SourceDict sources = containerDict();
    DataEngine::SourceDict::iterator it = sources.begin();
    if (m_waitingFor != 0) {
        scheduleSourcesUpdated();
    }

    m_waitingFor = 0;

    while (it != sources.end()) {
        m_waitingFor++;
        QString sensorName = it.key();
        KSGRD::SensorMgr->sendRequest( "localhost", sensorName, (KSGRD::SensorClient*)this, -1);
        ++it;
    }
}

void SystemMonitorEngine::answerReceived(int id, const QList<QByteArray> &answer)
{
    if (id < -1) {
        QStringList newSensorInfo = QString::fromUtf8(answer[0]).split('\t');
        QString sensorName = newSensorInfo[0];
        QString min = newSensorInfo[1];
        QString max = newSensorInfo[2];
        QString unit = newSensorInfo[3];
        DataEngine::SourceDict sources = containerDict();
        DataEngine::SourceDict::const_iterator it = sources.constFind(m_sensors.value(-id-2));

        if (it != sources.constEnd()) {
            it.value()->setData("name", sensorName);
            it.value()->setData("min", min);
            it.value()->setData("max", max);
            it.value()->setData("units", unit);
            scheduleSourcesUpdated();
        }

        return;
    }

    if (id == -1) {
        QStringList sensors;

        foreach (const QByteArray &sens, answer) {
            QStringList newSensorInfo = QString::fromUtf8(sens).split('\t');
            QString newSensor = newSensorInfo[0];
            sensors.append(newSensor);
            setData(newSensor, "value", QVariant());
            setData(newSensor, "type", newSensorInfo[1]);

            KSGRD::SensorMgr->sendRequest( "localhost", QString("%1?").arg(newSensor), (KSGRD::SensorClient*)this, -(sensors.indexOf(newSensor)+2));
        }

        m_sensors = sensors;
        return;
    }

    m_waitingFor--;
    QString reply;
    if (!answer.isEmpty()) {
        reply = QString::fromUtf8(answer[0]);
    }

    DataEngine::SourceDict sources = containerDict();
    DataEngine::SourceDict::const_iterator it = sources.constFind(m_sensors.value(id));
    if (it != sources.constEnd()) {
        it.value()->setData("value", reply);
    }

    if (m_waitingFor == 0) {
        scheduleSourcesUpdated();
    }
}

void SystemMonitorEngine::sensorLost( int )
{
    m_waitingFor--;
}
#include "systemmonitor.moc"

