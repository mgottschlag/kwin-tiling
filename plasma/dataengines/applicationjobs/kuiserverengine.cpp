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

static const int UPDATE_INTERVAL = 100;

JobView::JobView(QObject* parent)
    : Plasma::DataContainer(parent),
      m_capabilities(-1),
      m_percent(0),
      m_updateTimerId(0),
      m_speed(0),
      m_state(UnknownState),
      m_unitId(0)
{
    m_jobId = ++KuiserverEngine::s_jobId;
    setObjectName(QString("Job %1").arg(KuiserverEngine::s_jobId));

    new JobViewAdaptor(this);
    setSuspended(false);
}

JobView::~JobView()
{
    //kDebug();
}

uint JobView::jobId() const
{
    return m_jobId;
}

void JobView::scheduleUpdate()
{
    if (!m_updateTimerId) {
        m_updateTimerId = startTimer(UPDATE_INTERVAL);
    }
}

void JobView::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == m_updateTimerId) {
        killTimer(m_updateTimerId);
        m_updateTimerId = 0;
        checkForUpdate();

        if (m_state == Stopped) {
            emit becameUnused(objectName());
        }
    } else {
        Plasma::DataContainer::timerEvent(event);
    }
}

void JobView::terminate(const QString &errorMessage)
{
    setData("error", errorMessage);
    QTimer::singleShot(0, this, SLOT(finished()));
}

void JobView::finished()
{
    if (m_state != Stopped) {
        m_state = Stopped;
        setData("state", "stopped");
        setData("speed", QVariant());
        scheduleUpdate();
    }
}

void JobView::setSuspended(bool suspended)
{
    if (suspended) {
        if (m_state != Suspended) {
            m_state = Suspended;
            setData("state", "suspended");
            setData("speed", QVariant());
            scheduleUpdate();
        }
    } else if (m_state != Running) {
        m_state = Running;
        setData("state", "running");
        setData("speed", speedString());
        scheduleUpdate();
    }
}

int JobView::unitId(const QString &unit)
{
    int id = 0;
    if (m_unitMap.contains(unit)) {
        id = m_unitMap.value(unit);
    } else {
        id = m_unitId;
        setData(QString("totalUnit%1").arg(id), unit);
        setData(QString("totalAmount%1").arg(id), 0);
        setData(QString("processedUnit%1").arg(id), unit);
        setData(QString("processedAmount%1").arg(id), 0);
        m_unitMap.insert(unit, m_unitId);
        ++m_unitId;
        scheduleUpdate();
    }

    return id;
}

void JobView::setTotalAmount(qlonglong amount, const QString &unit)
{
    int id = unitId(unit);
    QString amountString = QString("totalAmount%1").arg(id);
    qlonglong prevTotal = data().value(amountString).toLongLong();
    if (prevTotal != amount) {
        if (m_speed > 0 && unit == "bytes") {
            QString processedString = QString("processedAmount%1").arg(id);
            qlonglong remaining = 1000 * (amount - data().value(processedString).toLongLong());
            setData("eta", remaining / m_speed);
        }

        setData(amountString, amount);
        scheduleUpdate();
    }
}

void JobView::setProcessedAmount(qlonglong amount, const QString &unit)
{
    int id = unitId(unit);
    QString processedString = QString("processedAmount%1").arg(id);
    qlonglong prevTotal = data().value(processedString).toLongLong();
    if (prevTotal != amount) {
        if (m_speed > 0 && unit == "bytes") {
            QString amountString = QString("totalAmount%1").arg(id);
            qlonglong remaining = 1000 * (data().value(amountString).toLongLong() - amount);
            setData("eta", remaining / m_speed);
        }

        setData(processedString, amount);
        scheduleUpdate();
    }
}

void JobView::setPercent(uint percent)
{
    if (m_percent != percent) {
        m_percent = percent;
        setData("percentage", m_percent);
        scheduleUpdate();
    }
}

void JobView::setSpeed(qlonglong bytesPerSecond)
{
    m_speed = bytesPerSecond;
}

QString JobView::speedString() const
{
    return i18nc("Byes per second", "%1/s", KGlobal::locale()->formatByteSize(m_speed));
}

void JobView::setInfoMessage(const QString &infoMessage)
{
    if (data().value("infoMessage") != infoMessage) {
        setData("infoMessage", infoMessage);
        scheduleUpdate();
    }
}

bool JobView::setDescriptionField(uint number, const QString &name, const QString &value)
{
    const QString labelString = QString("label%1").arg(number);
    const QString labelNameString = QString("labelName%1").arg(number);

    if (!data().contains(labelNameString) || data().value(labelString) != value) {
        setData(labelNameString, name);
        setData(labelString, value);
        scheduleUpdate();
    }

    return true;
}

void JobView::clearDescriptionField(uint number)
{
    const QString labelString = QString("label%1").arg(number);
    const QString labelNameString = QString("labelName%1").arg(number);

    setData(labelNameString, QVariant());
    setData(labelString, QVariant());
    scheduleUpdate();
}

void JobView::setAppName(const QString &appName)
{
    // don't need to update, this is only set once at creation
    setData("appName", appName);
}

void JobView::setAppIconName(const QString &appIconName)
{
    // don't need to update, this is only set once at creation
    setData("appIconName", appIconName);
}

void JobView::setCapabilities(int capabilities)
{
    if (m_capabilities != uint(capabilities)) {
        m_capabilities = capabilities;
        setData("suspendable", m_capabilities & KJob::Suspendable);
        setData("killable", m_capabilities & KJob::Killable);
        scheduleUpdate();
    }
}

QDBusObjectPath JobView::objectPath() const
{
    return m_objectPath;
}

void JobView::requestStateChange(State state)
{
    switch (state) {
        case Running:
            emit resumeRequested();
            break;
        case Suspended:
            emit suspendRequested();
            break;
        case Stopped:
            emit cancelRequested();
            break;
        default:
            break;
    }
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
}

QDBusObjectPath KuiserverEngine::requestView(const QString &appName,
                                             const QString &appIconName, int capabilities)
{
    JobView *jobView = new JobView(this);
    jobView->setAppName(appName);
    jobView->setAppIconName(appIconName);
    jobView->setCapabilities(capabilities);

    addSource(jobView);
    connect(jobView, SIGNAL(becameUnused(QString)), this, SLOT(removeSource(QString)));

    QDBusObjectPath path;
    path.setPath(QString("/JobViewServer/JobView_%1").arg(jobView->jobId()));
    QDBusConnection::sessionBus().registerObject(path.path(), jobView);

    return path;
}

Plasma::Service* KuiserverEngine::serviceForSource(const QString& source)
{
    JobView *jobView = qobject_cast<JobView *>(containerForSource(source));
    if (jobView) {
        return new JobControl(this, jobView);
    } else {
        return DataEngine::serviceForSource(source);
    }
}

void KuiserverEngine::init()
{
}

K_EXPORT_PLASMA_DATAENGINE(kuiserver, KuiserverEngine)

#include "kuiserverengine.moc"

