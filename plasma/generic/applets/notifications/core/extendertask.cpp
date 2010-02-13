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
#include <QtGui/QStyleOptionGraphicsItem>
#include <QtGui/QWidget> // QWIDGETSIZE_MAX

#include <plasma/extender.h>
#include <plasma/extenderitem.h>
#include <plasma/extendergroup.h>
#include <plasma/popupapplet.h>
#include <plasma/tooltipmanager.h>
#include <plasma/theme.h>

#include <KIcon>
#include <KGlobalSettings>

#include "../core/notificationsmanager.h"
#include "../core/job.h"
#include "../core/notification.h"
#include "../core/completedjobnotification.h"


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
    connect(manager, SIGNAL(notificationExpired(SystemTray::Notification*)),
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
        kWarning()<<360*(qreal)m_manager->jobTotals()->percentage()/100;
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing);
        QColor color = Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor);
        color.setAlphaF(0.6);
        painter->setPen(color);
        painter->drawArc(option->rect, 90*16, -(360*(qreal)m_manager->jobTotals()->percentage()/100)*16);
        painter->restore();
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
        switch (job->state()) {
            case Job::Running:
                ++runningJobs;
                break;
            case Job::Suspended:
                ++pausedJobs;
                break;
            default:
                break;
        }
    }

    int total = m_manager->jobs().count();

    foreach (Notification *notification, m_manager->notifications()) {
        if (qobject_cast<CompletedJobNotification *>(notification)) {
            ++completedJobs;
        } else if (!notification->isExpired()) {
            ++total;
        }
    }

    total += completedJobs;


    if (!(total + m_manager->notifications().count())) {
        m_systray->hidePopup();
    }

    if (!total) {
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

    if (completedJobs > 0) {
        tooltipContent += i18np("%1 completed job", "%1 completed jobs", completedJobs) + "<br>";
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

}

#include "extendertask.moc"
