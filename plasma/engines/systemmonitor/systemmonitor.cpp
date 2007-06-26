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


#include <KDebug>
#include <KLocale>

#include "plasma/datasource.h"

#include "sensormanager.h"

SystemMonitorEngine::SystemMonitorEngine(QObject* parent, const QStringList& args)
    : Plasma::DataEngine(parent)
{
    Q_UNUSED(args)
    m_timer = new QTimer(this);
    m_timer->setSingleShot(false);
    m_timer->start(2000); //every 2 seconds should be enough.
    connect(m_timer, SIGNAL(timeout()), this, SLOT(updateSensors()));

    KSGRD::SensorMgr = new KSGRD::SensorManager();
    KSGRD::SensorMgr->engage( "localhost", "", "ksysguardd" );

    m_waitingFor= 0;
    KSGRD::SensorMgr->sendRequest( "localhost", "monitors", (KSGRD::SensorClient*)this);
}

SystemMonitorEngine::~SystemMonitorEngine()
{
}

QStringList SystemMonitorEngine::sources() const { 
	return m_sensors; 
}
bool SystemMonitorEngine::sourceRequested(const QString &name)
{
    return true;
}

void SystemMonitorEngine::updateSensors()
{
    DataEngine::SourceDict sources = sourceDict();
    DataEngine::SourceDict::iterator it = sources.begin();
    if(m_waitingFor != 0)
        checkForUpdates();
    m_waitingFor = 0;
    while (it != sources.end()) {
        m_waitingFor++;
        QString sensorName = it.key();
	KSGRD::SensorMgr->sendRequest( "localhost", sensorName, (KSGRD::SensorClient*)this);
	++it;
    }
}

void SystemMonitorEngine::answerReceived( const QString &sensor, const QList<QByteArray>&answer ) {
    kDebug() << "ANSWER RECIEVED" << endl;
    if(sensor=="monitors") {
        QStringList sensors;
	foreach(QByteArray sens, answer) { 
		QStringList newSensorInfo = QString::fromUtf8(sens).split('\t');
		QString newSensor = newSensorInfo[0];
		sensors.append(newSensor);
	}
	emit newSource("hello");
	m_sensors = sensors;
	return;
    }

    m_waitingFor--;
    QString reply;
    if(!answer.isEmpty())
        reply = QString::fromUtf8(answer[0]);

    DataEngine::SourceDict sources = sourceDict();
    DataEngine::SourceDict::const_iterator it = sources.find(sensor);
    if (it != sources.constEnd())
        it.value()->setData("value", reply);

    if(m_waitingFor == 0)
        checkForUpdates();
}
void SystemMonitorEngine::sensorLost( const QString &) { 
    m_waitingFor--;
}
#include "systemmonitor.moc"
