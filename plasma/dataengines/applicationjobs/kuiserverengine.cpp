/*
 *   Copyright © 2008 Rob Scheepmaker <r.scheepmaker@student.utwente.nl>
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

#include "jobviewadaptor.h"
#include "jobviewserveradaptor.h"
#include "kuiserverengine.h"
#include "jobcontrol.h"

#include <QDBusConnection>

#include <KJob>

#include <Plasma/DataEngine>


uint KuiserverEngine::s_jobId = 0;

JobView::JobView(QObject* parent)
    : QObject(parent),
      m_state(Running)
{
    m_objectPath.setPath(QString("/JobViewServer/JobView_%1").arg(++KuiserverEngine::s_jobId));

    new JobViewAdaptor(this);
    QDBusConnection::sessionBus().registerObject(m_objectPath.path(), this);
    m_jobId = KuiserverEngine::s_jobId;
}

void JobView::terminate(const QString &errorMessage)
{
    m_error = errorMessage;
    m_state = Stopped;
    emit viewUpdated(this);
}

void JobView::setSuspended(bool suspended)
{
    if (suspended) {
        m_state = Suspended;
    } else {
        m_state = Running;
    }

    emit viewUpdated(this);
}

void JobView::setTotalAmount(qlonglong amount, const QString &unit)
{
    m_totalMap[unit] = amount;
    emit viewUpdated(this);
}

void JobView::setProcessedAmount(qlonglong amount, const QString &unit)
{
    m_processedMap[unit] = amount;
    emit viewUpdated(this);
}

void JobView::setPercent(uint percent)
{
    m_percent = percent;
    emit viewUpdated(this);
}

void JobView::setSpeed(qlonglong bytesPerSecond)
{
    m_speed = bytesPerSecond;
    emit viewUpdated(this);
}

QString JobView::speedString() const
{
    //FIXME: how to i18n this?
    return QString("%1/s").arg(KGlobal::locale()->formatByteSize(m_speed));
}

void JobView::setInfoMessage(const QString &infoMessage)
{
    m_infoMessage = infoMessage;
    emit viewUpdated(this);
}

bool JobView::setDescriptionField(uint number, const QString &name, const QString &value)
{
    m_labels[number] = value;
    m_labelNames[number] = name;
    emit viewUpdated(this);
    return true;
}

void JobView::clearDescriptionField(uint number)
{
    m_labels.remove(number);
    m_labelNames.remove(number);
}

void JobView::setAppName(const QString &appName)
{
    m_appName = appName;
}

void JobView::setAppIconName(const QString &appIconName)
{
    m_appIconName = appIconName;
}

void JobView::setCapabilities(int capabilities)
{
    m_capabilities = capabilities;
}

QString JobView::sourceName() const
{
    return QString("Job %1").arg(m_jobId);
}

QDBusObjectPath JobView::objectPath() const
{
    return m_objectPath;
}

KuiserverEngine::KuiserverEngine(QObject* parent, const QVariantList& args)
    : Plasma::DataEngine(parent, args)
{
    new JobViewServerAdaptor(this);

    QDBusConnection::sessionBus().registerService(QLatin1String("org.kde.JobViewServer"));
    QDBusConnection::sessionBus().registerObject(QLatin1String("/JobViewServer"), this);

    setMinimumPollingInterval(500);
}

KuiserverEngine::~KuiserverEngine()
{
    QDBusConnection::sessionBus().unregisterService("org.kde.JobViewServer");
    qDeleteAll(m_jobViews);
}

QDBusObjectPath KuiserverEngine::requestView(const QString &appName,
                                             const QString &appIconName, int capabilities)
{
    JobView *jobView = new JobView();
    connect(jobView, SIGNAL(viewUpdated(JobView*)),
            this, SLOT(sourceUpdated(JobView*)));

    jobView->setAppName(appName);
    jobView->setAppIconName(appIconName);
    jobView->m_appName = appName;
    jobView->m_appIconName = appIconName;
    jobView->m_capabilities = capabilities;

    m_jobViews[jobView->sourceName()] = jobView;
    return jobView->objectPath();
}

Plasma::Service* KuiserverEngine::serviceForSource(const QString& source)
{
    if (m_jobViews.contains(source)) {
        return new JobControl(this, m_jobViews[source]);
    } else {
        return DataEngine::serviceForSource(source);
    }
}

void KuiserverEngine::init()
{
}

void KuiserverEngine::sourceUpdated(JobView *jobView)
{
    QString sourceName = jobView->sourceName();

    Plasma::DataEngine::Data data;
    data["appName"] = jobView->m_appName;
    data["appIconName"] = jobView->m_appIconName;
    data["percentage"] = jobView->m_percent;
    data["suspendable"] = (jobView->m_capabilities & KJob::Suspendable);
    data["killable"] = (jobView->m_capabilities & KJob::Killable);
    data["infoMessage"] = jobView->m_infoMessage;

    if (!jobView->m_error.isEmpty()) {
        data["error"] = jobView->m_error;
    }

    if (jobView->m_state == JobView::Running) {
        data["speed"] = jobView->speedString();
    }

    for (int i = 0; i < jobView->m_labels.count(); i++) {
        data[QString("label%1").arg(i)] = jobView->m_labels[i];
        data[QString("labelName%1").arg(i)] = jobView->m_labelNames[i];
    }

    int i = 0;
    foreach (const QString &unit, jobView->m_totalMap.keys()) {
        data[QString("totalUnit%1").arg(i)] = unit;
        data[QString("totalAmount%1").arg(i++)] = jobView->m_totalMap[unit];
    }

    i = 0;
    foreach (const QString &unit, jobView->m_processedMap.keys()) {
        data[QString("processedUnit%1").arg(i)] = unit;
        data[QString("processedAmount%1").arg(i++)] = jobView->m_processedMap[unit];
    }

    switch (jobView->m_state) {
        case JobView::Running:
            data["state"] = "running";
            setData(sourceName, data);
            break;
        case JobView::Suspended:
            data["state"] = "suspended";
            setData(sourceName, data);
            break;
        case JobView::Stopped:
            data["state"] = "stopped";
            setData(sourceName, data);
            removeSource(sourceName);
            break;
    }

}

K_EXPORT_PLASMA_DATAENGINE(kuiserver, KuiserverEngine)

#include "kuiserverengine.moc"

