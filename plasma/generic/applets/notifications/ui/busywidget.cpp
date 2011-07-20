/***************************************************************************
 *   Copyright (C) 2008, 2009 Rob Scheepmaker <r.scheepmaker@student.utwente.nl> *
 *   Copyright (C) 2010 Marco Martin <notmart@gmail.com>                   *
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

#include "busywidget.h"
#include <fixx11h.h>

#include <QtGui/QPainter>
#include <QtGui/QTextOption>
#include <QtGui/QStyleOptionGraphicsItem>
#include <QtGui/QWidget> // QWIDGETSIZE_MAX
#include <QSequentialAnimationGroup>

#include <plasma/extender.h>
#include <plasma/extenderitem.h>
#include <plasma/extendergroup.h>
#include <plasma/popupapplet.h>
#include <plasma/tooltipmanager.h>
#include <plasma/theme.h>
#include <Plasma/Animation>
#include <Plasma/Animator>

#include <KIcon>
#include <KGlobalSettings>

#include "../core/notificationsmanager.h"
#include "../core/job.h"
#include "../core/notification.h"
#include "../core/completedjobnotification.h"



BusyWidget::BusyWidget(Plasma::PopupApplet *parent, const Manager *manager)
    : Plasma::BusyWidget(parent),
      m_icon("dialog-information"),
      m_state(Empty),
      m_svg(new Plasma::Svg(this)),
      m_systray(parent),
      m_manager(manager),
      m_total(0),
      m_suppressToolTips(false)
{
    setAcceptsHoverEvents(true);
    m_svg->setImagePath("icons/notification");
    m_svg->setContainsMultipleImages(true);
    setRunning(false);

    m_fadeInAnimation = Plasma::Animator::create(Plasma::Animator::PixmapTransitionAnimation);
    m_fadeInAnimation->setTargetWidget(this);
    m_fadeInAnimation->setProperty("duration", 1000);
    m_fadeInAnimation->setProperty("targetPixmap", m_svg->pixmap("notification-active"));

    m_fadeOutAnimation = Plasma::Animator::create(Plasma::Animator::PixmapTransitionAnimation);
    m_fadeOutAnimation->setTargetWidget(this);
    m_fadeOutAnimation->setProperty("duration", 1000);
    m_fadeOutAnimation->setProperty("startPixmap", m_svg->pixmap("notification-active"));


    m_fadeGroup = new QSequentialAnimationGroup(this);
    m_fadeGroup->addAnimation(m_fadeInAnimation);
    m_fadeGroup->addAnimation(m_fadeOutAnimation);

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

    Plasma::ToolTipManager::self()->registerWidget(this);
    updateTask();
}

void BusyWidget::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QRectF iconRect(0, 0, qMin(size().width(), size().height()), qMin(size().width(), size().height()));
    iconRect.moveCenter(boundingRect().center());

    if (m_state == Running) {
        const int arcStart = 90*16;
        const int arcEnd = -(360*(qreal)m_manager->jobTotals()->percentage()/100)*16;

        Plasma::BusyWidget::paint(painter, option, widget);

        //kDebug() << arcStart << arcEnd;

        QPixmap activePixmap(iconRect.size().toSize());
        activePixmap.fill(Qt::transparent);
        QPixmap inActivePixmap(iconRect.size().toSize());
        inActivePixmap.fill(Qt::transparent);
        QRect pieRect(QPoint(0, 0), activePixmap.size()*2);
        pieRect.moveCenter(activePixmap.rect().center());

        QPainter p(&activePixmap);
        p.setPen(Qt::NoPen);
        p.setBrush(Qt::black);
        p.setCompositionMode(QPainter::CompositionMode_Source);
        p.drawPie(pieRect, arcStart, arcEnd);
        p.setCompositionMode(QPainter::CompositionMode_SourceIn);
        m_svg->paint(&p, QRectF(QPointF(0, 0), iconRect.size()), "notification-progress-active");
        p.end();

        p.begin(&inActivePixmap);
        p.setPen(Qt::NoPen);
        p.setBrush(Qt::black);
        p.setCompositionMode(QPainter::CompositionMode_Source);
        p.drawPie(pieRect, arcStart, (360*16)+arcEnd);
        p.setCompositionMode(QPainter::CompositionMode_SourceIn);
        m_svg->paint(&p, QRectF(QPointF(0, 0), iconRect.size()), "notification-progress-inactive");
        p.end();

        painter->drawPixmap(iconRect.topLeft().toPoint(), activePixmap);
        painter->drawPixmap(iconRect.topLeft().toPoint(), inActivePixmap);

        Plasma::BusyWidget::paint(painter, option, widget);

    } else if (m_state == Empty && m_manager->notifications().count() > 0) {
        m_svg->paint(painter, iconRect, "notification-inactive");
    } else if (m_state == Empty && m_manager->notifications().count() == 0) {
        m_svg->paint(painter, iconRect, "notification-disabled");
    } else {
        // m_state ==  Info
        m_svg->paint(painter, iconRect, "notification-empty");
        QFont font(KGlobalSettings::smallestReadableFont());
        painter->setFont(font);
        QRectF r = rect();

        painter->setPen(Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor));

        const QFontMetrics fm(font);
        const QSize textSize = fm.boundingRect(label()).size();
        const bool textFits = textSize.width() <= r.width() && textSize.height() <= r.height();
        if (m_svg && m_svg->hasElement(expanderElement())) {
            QSizeF arrowSize(m_svg->elementSize(expanderElement()));
            QRectF arrowRect(r.center() - QPointF(arrowSize.width() / 2, arrowSize.height() + fm.xHeight() / 2), arrowSize);
            m_svg->paint(painter, arrowRect, expanderElement());

            r.setTop(arrowRect.bottom());

            if (textFits) {
                painter->drawText(r, Qt::AlignHCenter|Qt::AlignTop, label());
            }
        } else if (textFits) {
            painter->drawText(r, Qt::AlignCenter, label());
        }
    }

    if (m_fadeInAnimation->state() == QAbstractAnimation::Running) {
        QPixmap pix = m_fadeInAnimation->property("currentPixmap").value<QPixmap>();
        painter->drawPixmap(iconRect, pix, pix.rect());
    } else if (m_fadeOutAnimation->state() == QAbstractAnimation::Running) {
        QPixmap pix = m_fadeOutAnimation->property("currentPixmap").value<QPixmap>();
        painter->drawPixmap(iconRect, pix, pix.rect());
    }
}

void BusyWidget::resizeEvent(QGraphicsSceneResizeEvent *)
{
    //regenerate pixmaps
    m_svg->resize(contentsRect().size());
    m_fadeInAnimation->setProperty("targetPixmap", m_svg->pixmap("notification-active"));
    m_fadeOutAnimation->setProperty("startPixmap", m_svg->pixmap("notification-active"));
    m_svg->resize();
}

void BusyWidget::setState(State state)
{
    if (m_state == state) {
        return;
    }

    m_state = state;
    setRunning(m_state == Running);
    update();
}

QString BusyWidget::expanderElement() const
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

void BusyWidget::getJobCounts(int &runningJobs, int &pausedJobs, int &completedJobs)
{
    runningJobs = pausedJobs = completedJobs = 0;
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

}

void BusyWidget::updateTask()
{
    int runningJobs, pausedJobs, completedJobs;
    getJobCounts(runningJobs, pausedJobs, completedJobs);

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

    if (total > m_total) {
        m_fadeGroup->start();
    }
    m_total = total;

    if (!total) {
        setState(BusyWidget::Empty);
        setLabel(QString());
    } else if (runningJobs) {
        setState(BusyWidget::Running);
        setLabel(QString("%1").arg(QString::number(total)));
    } else {
        setState(BusyWidget::Info);
        setLabel(QString::number(total));
    }

    if (Plasma::ToolTipManager::self()->isVisible(this)) {
        toolTipAboutToShow();
    }
}

void BusyWidget::suppressToolTips(bool suppress)
{
    m_suppressToolTips = suppress;
}

void BusyWidget::toolTipAboutToShow()
{
    if (m_suppressToolTips) {
        Plasma::ToolTipManager::self()->setContent(this, Plasma::ToolTipContent());
        return;
    }

    int runningJobs, pausedJobs, completedJobs;
    getJobCounts(runningJobs, pausedJobs, completedJobs);

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


#include "busywidget.moc"
