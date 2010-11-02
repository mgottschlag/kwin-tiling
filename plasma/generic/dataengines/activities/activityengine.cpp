/*
 *   Copyright 2010 Chani Armitage <chani@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2 or
 *   (at your option) any later version.
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

#include "activityengine.h"
#include "activityservice.h"

#include <kactivitycontroller.h>
#include <kactivityinfo.h>

#include <QApplication>

ActivityEngine::ActivityEngine(QObject* parent, const QVariantList& args)
    : Plasma::DataEngine(parent, args)
{
    Q_UNUSED(args);
}

void ActivityEngine::init()
{
    if (qApp->applicationName() == "plasma-netbook") {
        //hack for the netbook
        //FIXME can I read a setting or something instead?
    } else {
        m_activityController = new KActivityController(this);
        QStringList activities = m_activityController->listActivities();
        //setData("allActivities", activities);
        foreach (const QString &id, activities) {
            insertActivity(id);
        }
        m_currentActivity = m_activityController->currentActivity();

        connect(m_activityController, SIGNAL(activityAdded(QString)), this, SLOT(activityAdded(QString)));
        connect(m_activityController, SIGNAL(activityRemoved(QString)), this, SLOT(activityRemoved(QString)));
        connect(m_activityController, SIGNAL(currentActivityChanged(QString)), this, SLOT(currentActivityChanged(QString)));

        //some convenience sources for times when checking every activity source would suck
        //it starts with _ so that it can easily be filtered out of sources()
        //maybe I should just make it not included in sources() instead?
        setData("_Convenience", "Current", m_currentActivity);
        setData("_Convenience", "Running", m_activityController->listActivities(KActivityInfo::Running));
    }
}

void ActivityEngine::insertActivity(const QString &id) {
    //id -> name, icon, state
    KActivityInfo *activity = new KActivityInfo(id, this);
    setData(id, "Name", activity->name());
    setData(id, "Icon", activity->icon());
    //TODO state
    setData(id, "Current", m_activityController->currentActivity() == id);
    connect(activity, SIGNAL(nameChanged(QString)), this, SLOT(activityNameChanged(QString)));
}

void ActivityEngine::activityAdded(const QString &id) {
    insertActivity(id);
    setData("_Convenience", "Running",
            m_activityController->listActivities(KActivityInfo::Running)); //FIXME horribly inefficient
}

void ActivityEngine::activityRemoved(const QString &id) {
    //FIXME delete the KActivityInfo
    removeSource(id);
    setData("_Convenience", "Running",
            m_activityController->listActivities(KActivityInfo::Running)); //FIXME horribly inefficient
}

void ActivityEngine::currentActivityChanged(const QString &id) {
    setData(m_currentActivity, "Current", false);
    m_currentActivity = id;
    setData(id, "Current", true);
    setData("_Convenience", "Current", id);
}

void ActivityEngine::activityNameChanged(const QString &newName)
{
    KActivityInfo *info = qobject_cast<KActivityInfo*>(sender());
    if (!info) {
        return;
    }
    setData(info->id(), "Name", newName);
}


Plasma::Service *ActivityEngine::serviceForSource(const QString &source)
{
    //FIXME validate the name
    ActivityService *service = new ActivityService(m_activityController, source);
    service->setParent(this);
    return service;
}

K_EXPORT_PLASMA_DATAENGINE(activities, ActivityEngine)

#include "activityengine.moc"
