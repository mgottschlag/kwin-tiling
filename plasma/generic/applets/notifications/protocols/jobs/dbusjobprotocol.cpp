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

#include <Plasma/DataEngineManager>
#include <Plasma/Service>
#include <Plasma/ServiceJob>

static const char engineName[] = "applicationjobs";

DBusJobProtocol::DBusJobProtocol(Manager *parent)
    : Protocol(parent),
      m_manager(parent),
      m_engine(0)
{
}


DBusJobProtocol::~DBusJobProtocol()
{
    if (m_engine) {
        Plasma::DataEngineManager::self()->unloadEngine(engineName);
    }

    foreach (DBusJob *job, m_jobs) {
        disconnect(job);
        job->destroy();
    }

    m_jobs.clear();
}


void DBusJobProtocol::init()
{
    m_engine = Plasma::DataEngineManager::self()->loadEngine(engineName);

    if (!m_engine->isValid()) {
        Plasma::DataEngineManager::self()->unloadEngine(engineName);
        m_engine = 0;
        return;
    }

    connect(m_engine, SIGNAL(sourceAdded(QString)),
            this, SLOT(prepareJob(QString)));
    connect(m_engine, SIGNAL(sourceRemoved(QString)),
            this, SLOT(removeJob(QString)));
}

void DBusJobProtocol::prepareJob(const QString &source)
{
    m_engine->connectSource(source, this);
}

void DBusJobProtocol::dataUpdated(const QString &source, const Plasma::DataEngine::Data &data)
{
    DBusJob *job = m_jobs.value(source, 0);

    if (!job) {
        job = new DBusJob(source, this);
        m_jobs.insert(source, job);
        connect(job, SIGNAL(jobDeleted(QString)),
                this, SLOT(removeJob(QString)));
        connect(job, SIGNAL(suspend(QString)),
                this, SLOT(suspend(QString)));
        connect(job, SIGNAL(resume(QString)),
                this, SLOT(resume(QString)));
        connect(job, SIGNAL(stop(QString)),
                this, SLOT(stop(QString)));
        connect(job, SIGNAL(ready(Job*)),
                this, SIGNAL(jobCreated(Job*)));
    }

    job->setApplicationName(data.value("appName").toString());
    job->setApplicationIconName(data.value("appIconName").toString());
    job->setPercentage(data["percentage"].toUInt());
    job->setError(data["error"].toString());
    job->setMessage(data["infoMessage"].toString());
    job->setSuspendable(data["suspendable"].toBool());
    job->setKillable(data["killable"].toBool());
    job->setSpeed(data["speed"].toString());
    job->setNumericSpeed(data["numericSpeed"].toLongLong());
    job->setEta(data["eta"].toULongLong());

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
}

void DBusJobProtocol::removeJob(const QString &source)
{
    if (m_jobs.contains(source)) {
        DBusJob *job = m_jobs.take(source);
        job->setState(Job::Stopped);
        job->destroy();
    }
}

void DBusJobProtocol::suspend(const QString &source)
{
    Plasma::Service *service = m_engine->serviceForSource(source);
    KConfigGroup op = service->operationDescription("suspend");
    KJob *job = service->startOperationCall(op);
    connect(job, SIGNAL(finished(KJob*)), service, SLOT(deleteLater()));
}

void DBusJobProtocol::resume(const QString &source)
{
    Plasma::Service *service = m_engine->serviceForSource(source);
    KConfigGroup op = service->operationDescription("resume");
    KJob *job = service->startOperationCall(op);
    connect(job, SIGNAL(finished(KJob*)), service, SLOT(deleteLater()));
}

void DBusJobProtocol::stop(const QString &source)
{
    Plasma::Service *service = m_engine->serviceForSource(source);
    KConfigGroup op = service->operationDescription("stop");
    KJob *job = service->startOperationCall(op);
    connect(job, SIGNAL(finished(KJob*)), service, SLOT(deleteLater()));
}

#include "dbusjobprotocol.moc"
