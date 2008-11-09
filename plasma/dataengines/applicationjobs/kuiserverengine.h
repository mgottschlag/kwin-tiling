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

#ifndef KUISERVERENGINE_H
#define KUISERVERENGINE_H

#include <QDBusObjectPath>

#include <plasma/dataengine.h>

class JobView;

namespace Plasma
{
    class Service;
} // namespace Plasma

class KuiserverEngine : public Plasma::DataEngine
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.JobViewServer")

public:
    KuiserverEngine(QObject* parent, const QVariantList& args);
    ~KuiserverEngine();

    QDBusObjectPath requestView(const QString &appName, const QString &appIconName,
                                int capabilities);
    Plasma::Service* serviceForSource(const QString& source);

    static uint s_jobId;

public Q_SLOTS:
    void sourceUpdated(JobView* jobView);

protected:
    void init();

private:
    QMap<QString, JobView*> m_jobViews;

};

class JobView : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.JobView")

public:
    enum State {
                 Running = 0,
                 Suspended = 1,
                 Stopped = 2
               };

    JobView(QObject *parent = 0);

    void setTotalAmount(qlonglong amount, const QString &unit);
    QString totalAmountSize() const;
    QString totalAmountFiles() const;

    void setProcessedAmount(qlonglong amount, const QString &unit);
    QString processedAmountSize() const;
    QString processedAmountFiles() const;

    void setSpeed(qlonglong bytesPerSecond);
    QString speedString() const;

    void setInfoMessage(const QString &infoMessage);
    QString infoMessage() const;

    bool setDescriptionField(uint number, const QString &name, const QString &value);
    void clearDescriptionField(uint number);

    void setAppName(const QString &appName);
    void setAppIconName(const QString &appIconName);
    void setCapabilities(int capabilities);
    void setPercent(uint percent);
    void setSuspended(bool suspended);

    void terminate(const QString &errorMessage);

    QString sourceName() const;

    QDBusObjectPath objectPath() const;

Q_SIGNALS:
    void suspendRequested();
    void resumeRequested();
    void cancelRequested();

    void viewUpdated(JobView* view);

private:
    QDBusObjectPath m_objectPath;

    uint m_capabilities;
    uint m_jobId;

    uint m_percent;

    qlonglong m_speed;
    qlonglong m_totalAmountSize;
    qlonglong m_totalAmountFiles;
    qlonglong m_processedAmountSize;
    qlonglong m_processedAmountFiles;

    State m_state;

    QString m_infoMessage;
    QString m_appName;
    QString m_appIconName;
    QString m_error;

    QMap<int, QString> m_labels;
    QMap<int, QString> m_labelNames;

    friend class KuiserverEngine;
    friend class JobAction;
};

#endif
