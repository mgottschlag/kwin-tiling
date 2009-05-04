/***************************************************************************
 *   Copyright (C) 2008, 2009 Rob Scheepmaker <r.scheepmaker@student.utwente.nl> *
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

#include <plasma/extender.h>
#include <plasma/extenderitem.h>
#include <plasma/extendergroup.h>
#include <plasma/popupapplet.h>
#include <plasma/tooltipmanager.h>
#include <plasma/widgets/busywidget.h>

#include <KIcon>


namespace SystemTray
{

class ExtenderTask::Private
{
public:
    Private(Plasma::PopupApplet *systemTray, Manager *manager,
            Plasma::Extender *extender, Task *q)
        : q(q),
          busyWidget(0),
          systemTray(systemTray),
          manager(manager),
          extender(extender)
    {
    }

    void updateTask()
    {
        int runningJobs = 0;
        int pausedJobs = 0;
        int completedJobs = 0;
        foreach (Job *job, manager->jobs()) {
            if (job->state() == Job::Running) {
                runningJobs++;
                break;
            }
            if (job->state() == Job::Suspended) {
                pausedJobs++;
                break;
            }
        }

        Plasma::ExtenderGroup *group = extender->group("completedJobsGroup");
        if (group) {
            completedJobs = group->items().count();
            group->setTitle(i18np("%1 recently completed job:", "%1 recently completed jobs:",
                            completedJobs));
        }
        int total= manager->jobs().count() + manager->notifications().count() + completedJobs;

        if (manager->jobs().isEmpty() &&
            manager->notifications().isEmpty() &&
            group && group->items().isEmpty()) {
            systemTray->hidePopup();
            delete q;
            return;
        } else if (runningJobs > 0) {
            busyWidget->setRunning(true);
            busyWidget->setLabel(QString("%1/%2").arg(QString::number(total - runningJobs))
                                                 .arg(QString::number(total)));
        } else {
            busyWidget->setRunning(false);
            busyWidget->setLabel(QString::number(total));
        }

        //make a nice plasma tooltip
        QString tooltipContent;
        if (runningJobs > 0) {
            tooltipContent += i18np("%1 running job", "%1 running jobs", runningJobs) + "<br>";
        }
        if (pausedJobs > 0) {
            tooltipContent += i18np("%1 suspended job", "%1 suspended jobs", pausedJobs) + "<br>";
        }
        if (!manager->notifications().isEmpty()) {
            tooltipContent += i18np("%1 notification", "%1 notifications",
                                    manager->notifications().count()) + "<br>";
        }

        Plasma::ToolTipContent data(i18n("Notifications and jobs"),
                                    tooltipContent,
                                    KIcon("help-about"));
        Plasma::ToolTipManager::self()->setContent(systemTray, data);
    }

    Task *q;
    QString typeId;
    QString iconName;
    QIcon icon;
    Plasma::BusyWidget *busyWidget;
    Plasma::PopupApplet *systemTray;
    Manager *manager;
    Plasma::Extender *extender;
};


ExtenderTask::ExtenderTask(Plasma::PopupApplet *systemTray, Manager *manager, 
                           Plasma::Extender *extender)
    : d(new Private(systemTray, manager, extender, this))
{
    setOrder(Last);

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
    connect(extender, SIGNAL(itemDetached(Plasma::ExtenderItem*)),
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
    Plasma::ToolTipManager::self()->unregisterWidget(d->systemTray);
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
    Q_UNUSED(host)
    return d->busyWidget;
}

}

#include "extendertask.moc"
