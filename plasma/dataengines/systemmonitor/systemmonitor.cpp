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
    KSGRD::SensorMgr->engage( "localhost", "", "ksysguardd" );

    m_waitingFor= 0;
    KSGRD::SensorMgr->sendRequest( "localhost", "monitors", (KSGRD::SensorClient*)this, -1);
}

SystemMonitorEngine::~SystemMonitorEngine()
{
}

QStringList SystemMonitorEngine::sources() const { 
    return m_sensors; 
}
bool SystemMonitorEngine::sourceRequestEvent(const QString &name)
{
    return false;
}
bool SystemMonitorEngine::updateSourceEvent(const QString &sensorName)
{
    KSGRD::SensorMgr->sendRequest( "localhost", sensorName, (KSGRD::SensorClient*)this, m_sensors.indexOf(sensorName));
    return false;
}

void SystemMonitorEngine::updateSensors()
{
    DataEngine::SourceDict sources = containerDict();
    DataEngine::SourceDict::iterator it = sources.begin();
    if(m_waitingFor != 0)
        scheduleSourcesUpdated();
    m_waitingFor = 0;
    while (it != sources.end()) {
        m_waitingFor++;
        QString sensorName = it.key();
	KSGRD::SensorMgr->sendRequest( "localhost", sensorName, (KSGRD::SensorClient*)this, -1);
	++it;
    }
}

void SystemMonitorEngine::answerReceived( int id, const QList<QByteArray>&answer ) {
    if(id==-1) {
        QStringList sensors;
	foreach(const QByteArray &sens, answer) { 
		QStringList newSensorInfo = QString::fromUtf8(sens).split('\t');
		QString newSensor = newSensorInfo[0];
		sensors.append(newSensor);
		setData(newSensor, "value", QVariant());
	}
	m_sensors = sensors;
	return;
    }
    m_waitingFor--;
    QString reply;
    if(!answer.isEmpty())
        reply = QString::fromUtf8(answer[0]);

    DataEngine::SourceDict sources = containerDict();
    DataEngine::SourceDict::const_iterator it = sources.constFind(m_sensors.value(id));
    if (it != sources.constEnd()) {
        it.value()->setData("value", reply);
    }

    if(m_waitingFor == 0)
        scheduleSourcesUpdated();
}
void SystemMonitorEngine::sensorLost( int ) { 
    m_waitingFor--;
}
#include "systemmonitor.moc"

