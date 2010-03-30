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



ExtenderTaskBusyWidget::ExtenderTaskBusyWidget(Plasma::PopupApplet *parent, const Manager *manager)
    : Plasma::BusyWidget(parent),
      m_icon("dialog-information"),
      m_state(Empty),
      m_svg(new Plasma::Svg(this)),
      m_systray(parent),
      m_manager(manager)
{
    setAcceptsHoverEvents(true);
    m_svg->setImagePath("widgets/notifications");
    m_svg->setContainsMultipleImages(true);
    setRunning(false);

    connect(manager, SIGNAL(notificationAdded(Notification*)),
            this, SLOT(updateTask()));
    connect(manager, SIGNAL(notificationRemoved(Notification*)),
            this, SLOT(updateTask()));
    connect(manager, SIGNAL(notificationChanged(Notification*)),
            this, SLOT(updateTask()));
    connect(manager, SIGNAL(notificationExpired(Notification*)),
            this, SLOT(updateTask()));
    connect(manager, SIGNAL(jobAdded(Job*)),
            this, SLOT(updateTask()));
    connect(manager, SIGNAL(jobRemoved(Job*)),
            this, SLOT(updateTask()));
    connect(manager, SIGNAL(jobStateChanged(Job*)),
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
    QRectF iconRect(0, 0, qMin(size().width(), size().height()), qMin(size().width(), size().height()));
    iconRect.moveCenter(boundingRect().center());

    if (m_state == Running) {
        const int arcStart = 90*16;
        const int arcEnd = -(360*(qreal)m_manager->jobTotals()->percentage()/100)*16;

        Plasma::BusyWidget::paint(painter, option, widget);

        //kWarning() << arcStart << arcEnd;

        QPixmap activePixmap(iconRect.size().toSize());
        activePixmap.fill(Qt::transparent);
        QPixmap inActivePixmap(iconRect.size().toSize());
        inActivePixmap.fill(Qt::transparent);

        QPainter p(&activePixmap);
        p.setPen(Qt::NoPen);
        p.setBrush(Qt::black);
        p.setCompositionMode(QPainter::CompositionMode_Source);
        p.drawPie(QRectF(QPointF(0, 0), iconRect.size()), arcStart, arcEnd);
        p.setCompositionMode(QPainter::CompositionMode_SourceIn);
        m_svg->paint(&p, QRectF(QPointF(0, 0), iconRect.size()), "notification-progress");
        p.end();

        p.begin(&inActivePixmap);
        p.setPen(Qt::NoPen);
        p.setBrush(Qt::black);
        p.setCompositionMode(QPainter::CompositionMode_Source);
        p.drawPie(QRectF(QPointF(0, 0), iconRect.size()), arcStart, (360*16)+arcEnd);
        p.setCompositionMode(QPainter::CompositionMode_SourceIn);
        m_svg->paint(&p, QRectF(QPointF(0, 0), iconRect.size()), "notification-empty");
        p.end();

        painter->drawPixmap(iconRect.topLeft().toPoint(), activePixmap);
        painter->drawPixmap(iconRect.topLeft().toPoint(), inActivePixmap);

        Plasma::BusyWidget::paint(painter, option, widget);

    } else if (m_state == Empty) {
        m_svg->paint(painter, iconRect, "notification-inactive");
    } else {
        // m_state ==  Info
        m_svg->paint(painter, iconRect, "notification-empty");
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
            return "expander-top";
        case Plasma::RightEdge:
            return "expander-right";
        case Plasma::LeftEdge:
            return "expander-left";
        case Plasma::BottomEdge:
        default:
            return "expander-bottom";
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
        tooltipContent += i18np("%1 running job", "%1 running jobs", runningJobs);
        if (pausedJobs > 0 || completedJobs > 0 || !m_manager->notifications().isEmpty()) {
            tooltipContent += "<br>";
        }
    }

    if (pausedJobs > 0) {
        tooltipContent += i18np("%1 suspended job", "%1 suspended jobs", pausedJobs);
        if (completedJobs > 0 || !m_manager->notifications().isEmpty()) {
            tooltipContent += "<br>";
        }
    }

    if (completedJobs > 0) {
        tooltipContent += i18np("%1 completed job", "%1 completed jobs", completedJobs);
        if (!m_manager->notifications().isEmpty()) {
            tooltipContent += "<br>";
        }
    }

    if (!m_manager->notifications().isEmpty()) {
        tooltipContent += i18np("%1 notification", "%1 notifications",
                                m_manager->notifications().count());
    }

    if (tooltipContent.isEmpty()) {
        tooltipContent = i18n("No active jobs or notifications");
    }

    Plasma::ToolTipContent data(i18n("Notifications and jobs"),
                                tooltipContent,
                                KIcon("help-about"));
    Plasma::ToolTipManager::self()->setContent(this, data);
}


#include "extendertask.moc"
