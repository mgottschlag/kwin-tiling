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
namespace DBus
{


class JobProtocol::Private
{
public:
    Private()
        : engine(0)
    {
    }

    Plasma::DataEngine *engine;
    QHash<QString, Job*> jobs;
};


static const char *engineName = "applicationjobs";


JobProtocol::JobProtocol(QObject *parent)
    : SystemTray::JobProtocol(parent),
      d(new JobProtocol::Private)
{
}


JobProtocol::~JobProtocol()
{
    if (d->engine) {
        Plasma::DataEngineManager::self()->unloadEngine(engineName);
    }

    delete d;
}


void JobProtocol::init()
{
    d->engine = Plasma::DataEngineManager::self()->loadEngine(engineName);

    if (!d->engine->isValid()) {
        d->engine = 0;
        return;
    }

    connect(d->engine, SIGNAL(sourceAdded(const QString&)),
            this, SLOT(prepareJob(const QString&)));
    connect(d->engine, SIGNAL(sourceRemoved(const QString&)),
            this, SLOT(removeJob(const QString&)));
}

void JobProtocol::prepareJob(const QString &source)
{
    d->engine->connectSource(source, this);
}


void JobProtocol::dataUpdated(const QString &source, const Plasma::DataEngine::Data &data)
{
    bool isNew = !d->jobs.contains(source);

    if (isNew) {
        d->jobs[source] = new Job(source, this);
        connect(d->jobs[source], SIGNAL(jobDeleted(const QString&)),
                this, SLOT(removeJob(const QString&)));
        connect(d->jobs[source], SIGNAL(suspend(const QString&)),
                this, SLOT(suspend(const QString&)));
        connect(d->jobs[source], SIGNAL(resume(const QString&)),
                this, SLOT(resume(const QString&)));
        connect(d->jobs[source], SIGNAL(stop(const QString&)),
                this, SLOT(stop(const QString&)));
    }

    Job* job = d->jobs[source];
    job->setApplicationName(data.value("appName").toString());
    job->setApplicationIconName(data.value("appIconName").toString());
    job->setPercentage(data["percentage"].toUInt());
    job->setError(data["error"].toString());
    job->setMessage(data["infoMessage"].toString());
    job->setSuspendable(data["suspendable"].toBool());
    job->setKillable(data["killable"].toBool());

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

    if (isNew) {
        emit jobCreated(job);
    } else {
        emit job->changed(job);
    }
}

void JobProtocol::removeJob(const QString &source)
{
    if (d->jobs.contains(source)) {
        d->jobs.take(source)->destroy();
    }
}

void JobProtocol::suspend(const QString &source)
{
    Plasma::Service *service = d->engine->serviceForSource(source);
    KConfigGroup op = service->operationDescription("suspend");
    service->startOperationCall(op);
}

void JobProtocol::resume(const QString &source)
{
    Plasma::Service *service = d->engine->serviceForSource(source);
    KConfigGroup op = service->operationDescription("resume");
    service->startOperationCall(op);
}

void JobProtocol::stop(const QString &source)
{
    Plasma::Service *service = d->engine->serviceForSource(source);
    KConfigGroup op = service->operationDescription("stop");
    service->startOperationCall(op);
}

}
}

#include "dbusjobprotocol.moc"
