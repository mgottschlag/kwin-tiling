/***************************************************************************
 *   Copyright 2008 by Rob Scheepmaker <r.scheepmaker@student.utwente.nl>  *
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

#include "jobwidget.h"
#include "../core/job.h"

#include <KGlobalSettings>

#include <QFont>
#include <QAction>

#include <plasma/widgets/meter.h>
#include <Plasma/DataEngine>
#include <Plasma/Service>
#include <Plasma/ExtenderItem>
#include <Plasma/Theme>

JobWidget::JobWidget(SystemTray::Job *job, Plasma::ExtenderItem *parent)
    : Plasma::Meter(parent),
    m_extenderItem(parent),
    m_job(job)
{
    Q_ASSERT(m_extenderItem);

    setMeterType(Plasma::Meter::BarMeterHorizontal);
    setSvg("systemtray/bar_meter_horizontal");
    setMaximum(100);

    setMinimumWidth(300);

    Plasma::Theme *theme = Plasma::Theme::defaultTheme();

    QColor color = theme->color(Plasma::Theme::TextColor);
    QFont font = theme->font(Plasma::Theme::DefaultFont);
    for (int i = 0; i < 7; i++) {
        setLabelColor(i, color);
        setLabelFont(i, font);
    }

    setLabelAlignment(0, Qt::AlignRight);
    setLabelAlignment(1, Qt::AlignLeft);
    setLabelAlignment(2, Qt::AlignRight);
    setLabelAlignment(3, Qt::AlignLeft);
    setLabelAlignment(4, Qt::AlignRight);
    setLabelAlignment(5, Qt::AlignLeft);
    setLabelAlignment(6, Qt::AlignRight);

    setValue(0);

    if (m_job) {
        connect(m_job, SIGNAL(changed()),
                this, SLOT(updateJob()));
        connect(m_job, SIGNAL(destroyed()),
                this, SLOT(destroy()));

        //the suspend action
        QAction *suspendAction = new QAction(m_extenderItem);
        suspendAction->setIcon(KIcon("media-playback-pause"));
        suspendAction->setEnabled(true);
        suspendAction->setVisible(false);
        suspendAction->setToolTip(i18n("Pause job"));
        m_extenderItem->addAction("suspend", suspendAction);
        connect(suspendAction, SIGNAL(triggered()), m_job,
                SLOT(suspend()));

        //the resume action
        QAction *resumeAction = new QAction(m_extenderItem);
        resumeAction->setIcon(KIcon("media-playback-start"));
        resumeAction->setEnabled(true);
        resumeAction->setVisible(false);
        resumeAction->setToolTip(i18n("Resume job"));
        m_extenderItem->addAction("resume", resumeAction);
        connect(resumeAction, SIGNAL(triggered()), m_job,
                SLOT(resume()));

        //the friendly stop action
        QAction *stopAction = new QAction(m_extenderItem);
        stopAction->setIcon(KIcon("media-playback-stop"));
        stopAction->setEnabled(true);
        stopAction->setVisible(true);
        stopAction->setToolTip(i18n("Cancel job"));
        m_extenderItem->addAction("stop", stopAction);
        connect(stopAction, SIGNAL(triggered()), m_job,
                SLOT(stop()));

        updateJob();
    } else {
        m_extenderItem->showCloseButton();

        labelName0 = m_extenderItem->config().readEntry("labelName0", "");
        label0= m_extenderItem->config().readEntry("label0", "");
        labelName1 = m_extenderItem->config().readEntry("labelName1", "");
        label1 = m_extenderItem->config().readEntry("label1", "");

        updateLabels();
    }
}

JobWidget::~JobWidget()
{
}

void JobWidget::destroy()
{
    if (!m_extenderItem->isDetached()) {
        m_extenderItem->setAutoExpireDelay(6000);
        m_extenderItem->setTitle(i18nc("%1 is the name of the job, can be things like Copying, deleting, moving", "(finished) %1", m_job->message()));
        updateJob();
        setValue(100);
    }
}

void JobWidget::updateJob()
{
    setValue(m_job->percentage());

    Plasma::ExtenderItem *item = m_extenderItem;

    if (m_job) {
        if (m_job->labels().count() > 0) {
            labelName0 = m_job->labels().value(0).first;
            label0 = m_job->labels().value(0).second;
        }
        if (m_job->labels().count() > 1) {
            labelName1 = m_job->labels().value(1).first;
            label1 = m_job->labels().value(1).second;
        }
        KConfigGroup cg = m_extenderItem->config();
        cg.writeEntry("labelName0", labelName0);
        cg.writeEntry("label0", label0);
        cg.writeEntry("labelName1", labelName1);
        cg.writeEntry("label1", label1);
    }

    updateLabels();

    //show the current status in the title.
    if (!m_job->error().isEmpty()) {
        item->setTitle(m_job->error());
    } else if (m_job->state() == SystemTray::Job::Running) {
        //FIXME: howto i18n this
        item->setTitle(m_job->message());
    } else if (m_job->state() == SystemTray::Job::Suspended) {
        item->setTitle(i18nc("%1 is the name of the job, can be things like Copying, deleting, moving", "(paused) %1", m_job->message()));
    } else {
        item->setTitle(i18nc("%1 is the name of the job, can be things like Copying, deleting, moving", "(finished) %1", m_job->message()));
    }

    //set the correct actions to visible.
    if (item->action("suspend")) {
        item->action("suspend")->setVisible(m_job->isSuspendable() &&
                                            m_job->state() == SystemTray::Job::Running);
    }
    if (item->action("resume")) {
        item->action("resume")->setVisible(m_job->isSuspendable() &&
                                           m_job->state() == SystemTray::Job::Suspended);
    }
    if (item->action("stop")) {
        item->action("stop")->setVisible(m_job->isKillable());
    }

    setLabel(4, m_job->speed());
    setLabel(5, KGlobal::locale()->formatByteSize(m_job->processedAmounts()["bytes"]));
    setLabel(6, KGlobal::locale()->formatByteSize(m_job->totalAmounts()["bytes"]));

    item->setIcon(m_job->applicationIconName());
}

void JobWidget::resizeEvent(QGraphicsSceneResizeEvent *event)
{
    Meter::resizeEvent(event);
    updateLabels();
}

void JobWidget::updateLabels()
{
    //TODO: not base this on the size, but on the size of the label in question.
    Plasma::Theme *theme = Plasma::Theme::defaultTheme();
    QFont font = theme->font(Plasma::Theme::DefaultFont);
    QFontMetricsF fm(font);
    if (!labelName0.isEmpty()) {
        setLabel(0, QString("%1: ").arg(labelName0));
    }
    setLabel(1, fm.elidedText(label0, Qt::ElideMiddle, labelRect(1).width()));

    if (!labelName1.isEmpty()) {
        setLabel(2, QString("%1: ").arg(labelName1));
    }
    setLabel(3, fm.elidedText(label1, Qt::ElideMiddle, labelRect(3).width()));
}

#include "jobwidget.moc"

