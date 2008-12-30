/***************************************************************************
 *   Copyright (C) 2008 Rob Scheepmaker <r.scheepmaker@student.utwente.nl> *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#include "dbusjob.h"
#include "dbusjobprotocol.h"


#include <KConfigGroup>
#include <KIcon>

#include <plasma/dataenginemanager.h>
#include <plasma/service.h>


namespace SystemTray
{

static const char *engineName = "applicationjobs";

DBusJobProtocol::DBusJobProtocol(QObject *parent)
    : Protocol(parent),
      m_engine(0)
{
}


DBusJobProtocol::~DBusJobProtocol()
{
    if (m_engine) {
        Plasma::DataEngineManager::self()->unloadEngine(engineName);
    }
}


void DBusJobProtocol::init()
{
    m_engine = Plasma::DataEngineManager::self()->loadEngine(engineName);

    if (!m_engine->isValid()) {
        m_engine = 0;
        return;
    }

    connect(m_engine, SIGNAL(sourceAdded(const QString&)),
            this, SLOT(prepareJob(const QString&)));
    connect(m_engine, SIGNAL(sourceRemoved(const QString&)),
            this, SLOT(removeJob(const QString&)));
}

void DBusJobProtocol::prepareJob(const QString &source)
{
    m_engine->connectSource(source, this);
}


void DBusJobProtocol::dataUpdated(const QString &source, const Plasma::DataEngine::Data &data)
{
    bool isNew = !m_jobs.contains(source);

    if (isNew) {
        m_jobs[source] = new DBusJob(source, this);
        connect(m_jobs[source], SIGNAL(jobDeleted(const QString&)),
                this, SLOT(removeJob(const QString&)));
        connect(m_jobs[source], SIGNAL(suspend(const QString&)),
                this, SLOT(suspend(const QString&)));
        connect(m_jobs[source], SIGNAL(resume(const QString&)),
                this, SLOT(resume(const QString&)));
        connect(m_jobs[source], SIGNAL(stop(const QString&)),
                this, SLOT(stop(const QString&)));
        connect(m_jobs[source], SIGNAL(ready(SystemTray::Job*)),
                this, SIGNAL(jobCreated(SystemTray::Job*)));
    }

    DBusJob* job = m_jobs[source];

    job->setApplicationName(data.value("appName").toString());
    job->setApplicationIconName(data.value("appIconName").toString());
    job->setPercentage(data["percentage"].toUInt());
    job->setError(data["error"].toString());
    job->setMessage(data["infoMessage"].toString());
    job->setSuspendable(data["suspendable"].toBool());
    job->setKillable(data["killable"].toBool());
    job->setSpeed(data["speed"].toString());

    if (data["state"].toString() == "running") {
        job->setState(Job::Running);
    } else if (data["state"].toString() == "suspended") {
        job->setState(Job::Suspended);
    } else {
        job->setState(Job::Stopped);
    }

    int i = 0;
    QList<QPair<QString, QString> > labels;
    while (data.contains(QString("label%1").arg(i))) {
        QPair<QString, QString> label;
        label.first = data[QString("labelName%1").arg(i)].toString();
        label.second = data[QString("label%1").arg(i)].toString();
        labels << label;
        i++;
    }
    job->setLabels(labels);

    i = 0;
    QMap<QString, qlonglong> totalAmounts;
    while (data.contains(QString("totalUnit%1").arg(i))) {
        QString unit = data[QString("totalUnit%1").arg(i)].toString();
        qlonglong amount = data[QString("totalAmount%1").arg(i)].toLongLong();
        totalAmounts[unit] = amount;
        i++;
    }
    job->setTotalAmounts(totalAmounts);

    i = 0;
    QMap<QString, qlonglong> processedAmounts;
    while (data.contains(QString("processedUnit%1").arg(i))) {
        QString unit = data[QString("processedUnit%1").arg(i)].toString();
        qlonglong amount = data[QString("processedAmount%1").arg(i)].toLongLong();
        processedAmounts[unit] = amount;
        i++;
    }
    job->setProcessedAmounts(processedAmounts);

    if (!isNew) {
        emit job->changed(job);
    }
}

void DBusJobProtocol::removeJob(const QString &source)
{
    if (m_jobs.contains(source)) {
        m_jobs[source]->setState(Job::Stopped);
        emit m_jobs[source]->changed(m_jobs[source]);
        m_jobs.take(source)->destroy();
    }
}

void DBusJobProtocol::suspend(const QString &source)
{
    Plasma::Service *service = m_engine->serviceForSource(source);
    KConfigGroup op = service->operationDescription("suspend");
    service->startOperationCall(op);
}

void DBusJobProtocol::resume(const QString &source)
{
    Plasma::Service *service = m_engine->serviceForSource(source);
    KConfigGroup op = service->operationDescription("resume");
    service->startOperationCall(op);
}

void DBusJobProtocol::stop(const QString &source)
{
    Plasma::Service *service = m_engine->serviceForSource(source);
    KConfigGroup op = service->operationDescription("stop");
    service->startOperationCall(op);
}

}

#include "dbusjobprotocol.moc"
