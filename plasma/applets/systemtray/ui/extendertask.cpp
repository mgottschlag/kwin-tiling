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

#include "extendertask.h"
#include <fixx11h.h>

#include <QtGui/QPainter>
#include <QtGui/QTextOption>
#include <QtGui/QWidget> // QWIDGETSIZE_MAX

#include <plasma/extender.h>
#include <plasma/extenderitem.h>
#include <plasma/extendergroup.h>
#include <plasma/popupapplet.h>
#include <plasma/tooltipmanager.h>
#include <plasma/widgets/busywidget.h>

#include <KIcon>
#include <KGlobalSettings>

#include "../core/manager.h"
#include "../core/job.h"


namespace SystemTray
{

class ExtenderTaskBusyWidget : public Plasma::BusyWidget
{
public:
    enum State { Empty, Info, Running };

    ExtenderTaskBusyWidget(Plasma::PopupApplet *parent)
        : Plasma::BusyWidget(parent),
          m_icon("dialog-information"),
          m_state(Empty),
          m_svg(new Plasma::Svg(this)),
          m_systray(parent)
    {
        setAcceptsHoverEvents(true);
        m_svg->setImagePath("widgets/tasks");
        setRunning(false);
    }

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0)
    {
        if (m_state == Running) {
            Plasma::BusyWidget::paint(painter, option, widget);
            return;
        }

        if (m_state == Empty) {
            //TODO: something nicer perhaps .. right now we just paint a centered, disabled 'i' icon
            QPixmap pixmap = m_icon.pixmap(size().toSize(), QIcon::Disabled);
            QPoint p(qMax(qreal(0), (size().width() - pixmap.width()) / 2),
                    qMax(qreal(0), (size().height() - pixmap.height()) / 2));
            painter->drawPixmap(p, pixmap);

            return;
        }

        QFont font(KGlobalSettings::smallestReadableFont());
        painter->setFont(font);
        QRectF r = rect();

        if (m_svg && m_svg->hasElement(expanderElement())) {
            QFontMetrics fm(font);
            QSizeF arrowSize(m_svg->elementSize(expanderElement()));
            QRectF arrowRect(r.center() - QPointF(arrowSize.width() / 2, arrowSize.height() + fm.xHeight() / 2), arrowSize);
            m_svg->paint(painter, arrowRect, expanderElement());

            r.setTop(arrowRect.bottom());
            painter->drawText(r, Qt::AlignHCenter|Qt::AlignTop, label());
        } else {
            painter->drawText(r, Qt::AlignCenter, label());
        }
    }

    void setState(State state)
    {
        if (m_state == state) {
            return;
        }

        m_state = state;
        setRunning(m_state == Running);
        update();
    }

    QString expanderElement() const
    {
        switch (m_systray->location()) {
            case Plasma::TopEdge:
                return "group-expander-top";
            case Plasma::RightEdge:
                return "group-expander-right";
            case Plasma::LeftEdge:
                return "group-expander-left";
            case Plasma::BottomEdge:
            default:
                return "group-expander-bottom";
        }
    }

private:
    KIcon m_icon;
    State m_state;
    Plasma::Svg *m_svg;
    Plasma::PopupApplet *m_systray;
};

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
            } else if (job->state() == Job::Suspended) {
                pausedJobs++;
            }
        }

        Plasma::ExtenderGroup *group = extender->group("completedJobsGroup");
        if (group) {
            completedJobs = group->items().count();
            group->setTitle(i18np("%1 Recently Completed Job:", "%1 Recently Completed Jobs:",
                            completedJobs));
        }

        int total = manager->jobs().count() + manager->notifications().count() + completedJobs;

        if (!total) {
            systemTray->hidePopup();
            busyWidget->setState(ExtenderTaskBusyWidget::Empty);
            busyWidget->setLabel(QString());
        } else if (runningJobs) {
            busyWidget->setState(ExtenderTaskBusyWidget::Running);
            busyWidget->setLabel(QString("%1/%2").arg(QString::number(total - runningJobs))
                                                 .arg(QString::number(total)));
        } else {
            busyWidget->setState(ExtenderTaskBusyWidget::Info);
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

        if (tooltipContent.isEmpty()) {
            tooltipContent = i18n("No active jobs or notifications");
        }

        Plasma::ToolTipContent data(i18n("Notifications and jobs"),
                                    tooltipContent,
                                    KIcon("help-about"));
        Plasma::ToolTipManager::self()->setContent(busyWidget, data);
    }

    Task *q;
    QString typeId;
    QString iconName;
    QIcon icon;
    ExtenderTaskBusyWidget *busyWidget;
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

    d->busyWidget = new ExtenderTaskBusyWidget(systemTray);
    d->busyWidget->setMinimumSize(22, 22);
    d->busyWidget->setMaximumSize(26, QWIDGETSIZE_MAX);
    connect(d->busyWidget, SIGNAL(clicked()), systemTray, SLOT(togglePopup()));

    d->updateTask();
}


ExtenderTask::~ExtenderTask()
{
    emit taskDeleted(d->typeId);
    delete d->busyWidget;
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
