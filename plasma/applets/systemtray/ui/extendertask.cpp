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

#include "../core/manager.h"
#include "../core/job.h"
#include "extendertask.h"
#include <fixx11h.h>

#include <QtGui/QWidget> // QWIDGETSIZE_MAX
#include <QtGui/QTextOption>

#include <plasma/popupapplet.h>
#include <plasma/widgets/busywidget.h>


namespace SystemTray
{

class ExtenderTask::Private
{
public:
    Private(Plasma::PopupApplet *systemTray, Manager *manager, Task *q)
        : q(q),
          busyWidget(0),
          systemTray(systemTray),
          manager(manager)
    {
    }

    void updateTask()
    {
        int runningJobs = 0;
        foreach (Job *job, manager->jobs()) {
            if (job->state() == Job::Running) {
                runningJobs++;
                break;
            }
        }

        int total= manager->jobs().count() + manager->notifications().count();

        if (manager->jobs().isEmpty() &&
            manager->notifications().isEmpty()) {
            systemTray->hidePopup();
            delete q;
        } else if (runningJobs > 0) {
            busyWidget->setRunning(true);
            busyWidget->setLabel(QString("%1/%2").arg(QString::number(total - runningJobs))
                                                 .arg(QString::number(total)));
        } else {
            busyWidget->setRunning(false);
            busyWidget->setLabel(QString::number(total));
        }
    }

    Task *q;
    QString typeId;
    QString iconName;
    QIcon icon;
    Plasma::BusyWidget *busyWidget;
    Plasma::PopupApplet *systemTray;
    Manager *manager;
};


ExtenderTask::ExtenderTask(Plasma::PopupApplet *systemTray, Manager *manager)
    : d(new Private(systemTray, manager, this))
{
    setOrder(Last);

    //FIXME: one signal in the manager maybe?
    connect(manager, SIGNAL(notificationAdded(SystemTray::Notification*)),
            this, SLOT(updateTask()));
    connect(manager, SIGNAL(notificationRemoved(SystemTray::Notification*)),
            this, SLOT(updateTask()));
    connect(manager, SIGNAL(notificationChanged(SystemTray::Notification*)),
            this, SLOT(updateTask()));
    connect(manager, SIGNAL(jobAdded(SystemTray::Job*)),
            this, SLOT(updateTask()));
    connect(manager, SIGNAL(jobRemoved(SystemTray::Job*)),
            this, SLOT(updateTask()));
    connect(manager, SIGNAL(jobChanged(SystemTray::Job*)),
            this, SLOT(updateTask()));

    d->busyWidget = new Plasma::BusyWidget(systemTray);
    d->busyWidget->setMinimumSize(22, 22);
    d->busyWidget->setMaximumSize(26, QWIDGETSIZE_MAX);
    connect(d->busyWidget, SIGNAL(clicked()), systemTray, SLOT(togglePopup()));

    d->updateTask();
}


ExtenderTask::~ExtenderTask()
{
    emit taskDeleted(d->typeId);
    delete d;
}


bool ExtenderTask::isEmbeddable() const
{
    return true;
}

bool ExtenderTask::isValid() const
{
    return true;
}

bool ExtenderTask::isHideable() const
{
    return false;
}

QString ExtenderTask::name() const
{
    return i18n("Show or hide notifications and jobs");
}


QString ExtenderTask::typeId() const
{
    //FIXME: what should we return here?
    return "toggle_extender";
}


QIcon ExtenderTask::icon() const
{
    return d->icon;
}

void ExtenderTask::setIcon(const QString &icon)
{
    d->iconName = icon;
}

QGraphicsWidget* ExtenderTask::createWidget(Plasma::Applet *host)
{
    return d->busyWidget;
}

}

#include "extendertask.moc"
