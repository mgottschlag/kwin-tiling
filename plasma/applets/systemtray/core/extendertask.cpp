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
#include <plasma/theme.h>

#include <KIcon>
#include <KGlobalSettings>

#include "../core/manager.h"
#include "../core/job.h"


namespace SystemTray
{


ExtenderTaskBusyWidget::ExtenderTaskBusyWidget(Plasma::PopupApplet *parent, const Manager *manager)
    : Plasma::BusyWidget(parent),
      m_icon("dialog-information"),
      m_state(Empty),
      m_svg(new Plasma::Svg(this)),
      m_systray(parent),
      m_manager(manager)
{
    setAcceptsHoverEvents(true);
    m_svg->setImagePath("widgets/tasks");
    setRunning(false);

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
    connect(manager, SIGNAL(jobStateChanged(SystemTray::Job*)),
            this, SLOT(updateTask()));

    Plasma::Extender *extender = qobject_cast<Plasma::Extender *>(m_systray->graphicsWidget());
    if (extender) {
        connect(extender, SIGNAL(itemDetached(Plasma::ExtenderItem*)),
                this, SLOT(updateTask()));
    }

    updateTask();
}

void ExtenderTaskBusyWidget::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    if (m_state == Running) {
        Plasma::BusyWidget::paint(painter, option, widget);
    } else if (m_state == Empty) {
        //TODO: something nicer perhaps .. right now we just paint a centered, disabled 'i' icon
        QPixmap pixmap = m_icon.pixmap(size().toSize(), QIcon::Disabled);
        QPoint p(qMax(qreal(0), (size().width() - pixmap.width()) / 2),
                 qMax(qreal(0), (size().height() - pixmap.height()) / 2));
        painter->drawPixmap(p, pixmap);
    } else {
        // m_state ==  Info
        QFont font(KGlobalSettings::smallestReadableFont());
        painter->setFont(font);
        QRectF r = rect();

        painter->setPen(Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor));

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
}

void ExtenderTaskBusyWidget::setState(State state)
{
    if (m_state == state) {
        return;
    }

    m_state = state;
    setRunning(m_state == Running);
    update();
}

QString ExtenderTaskBusyWidget::expanderElement() const
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

void ExtenderTaskBusyWidget::updateTask()
{
    int runningJobs = 0;
    int pausedJobs = 0;
    int completedJobs = 0;
    foreach (const Job *job, m_manager->jobs()) {
        if (job->state() == Job::Running) {
            runningJobs++;
        } else if (job->state() == Job::Suspended) {
            pausedJobs++;
        }
    }

    //FIXME: assumptions++
    Plasma::Extender *extender = qobject_cast<Plasma::Extender *>(m_systray->graphicsWidget());
    if (extender) {
        Plasma::ExtenderGroup *group = extender->group("completedJobsGroup");
        if (group) {
            completedJobs = group->items().count();
            group->setTitle(i18np("%1 Recently Completed Job:", "%1 Recently Completed Jobs:",
                           completedJobs));
        }
    }

    int total = m_manager->jobs().count() + m_manager->notifications().count() + completedJobs;

    if (!total) {
        m_systray->hidePopup();
        setState(ExtenderTaskBusyWidget::Empty);
        setLabel(QString());
    } else if (runningJobs) {
        setState(ExtenderTaskBusyWidget::Running);
        setLabel(QString("%1/%2").arg(QString::number(total - runningJobs))
                                 .arg(QString::number(total)));
    } else {
        setState(ExtenderTaskBusyWidget::Info);
        setLabel(QString::number(total));
    }

    //make a nice plasma tooltip
    QString tooltipContent;
    if (runningJobs > 0) {
        tooltipContent += i18np("%1 running job", "%1 running jobs", runningJobs) + "<br>";
    }

    if (pausedJobs > 0) {
        tooltipContent += i18np("%1 suspended job", "%1 suspended jobs", pausedJobs) + "<br>";
    }

    if (!m_manager->notifications().isEmpty()) {
        tooltipContent += i18np("%1 notification", "%1 notifications",
                                m_manager->notifications().count()) + "<br>";
    }

    if (tooltipContent.isEmpty()) {
        tooltipContent = i18n("No active jobs or notifications");
    }

    Plasma::ToolTipContent data(i18n("Notifications and jobs"),
                                tooltipContent,
                                KIcon("help-about"));
    Plasma::ToolTipManager::self()->setContent(this, data);
}

class ExtenderTask::Private
{
public:
    Private(const Manager *manager, Task *q)
        : q(q),
          manager(manager)
    {
    }

    Task *q;
    QString typeId;
    QString iconName;
    QIcon icon;
    const Manager *manager;
};

ExtenderTask::ExtenderTask(const Manager *manager)
    : Task(const_cast<Manager *>(manager)), //FIXME: that's a little ugly :)
      d(new Private(manager, this))
{
    setIcon("help-about");
    setOrder(Last);
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
    //Assumption on the systray being a popupapplet
    Plasma::PopupApplet *popupApplet = static_cast<Plasma::PopupApplet *>(host);
    ExtenderTaskBusyWidget *busyWidget = new ExtenderTaskBusyWidget(popupApplet, d->manager);
    busyWidget->setMinimumSize(8, 8);
    busyWidget->setPreferredSize(22, 22);
    connect(busyWidget, SIGNAL(clicked()), popupApplet, SLOT(togglePopup()));
    return busyWidget;
}

}

#include "extendertask.moc"
