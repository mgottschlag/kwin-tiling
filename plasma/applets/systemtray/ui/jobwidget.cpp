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
#include <QGraphicsGridLayout>
#include <QLabel>

#include <plasma/widgets/meter.h>
#include <Plasma/DataEngine>
#include <Plasma/Service>
#include <Plasma/ExtenderItem>
#include <Plasma/Theme>
#include <Plasma/Label>
#include <Plasma/Meter>

JobWidget::JobWidget(SystemTray::Job *job, Plasma::ExtenderItem *parent)
    : QGraphicsWidget(parent),
    m_extenderItem(parent),
    m_job(job)
{
    Q_ASSERT(m_extenderItem);

    m_meter = new Plasma::Meter(this);
    m_meter->setSvg("widgets/bar_meter_horizontal");
    m_meter->setMeterType(Plasma::Meter::BarMeterHorizontal);
    m_meter->setMaximumHeight(16);
    m_meter->setMaximum(100);
    m_meter->setValue(0);

    m_fromNameLabel = new Plasma::Label(this);
    m_fromLabel = new Plasma::Label(this);
    m_toNameLabel = new Plasma::Label(this);
    m_toLabel = new Plasma::Label(this);
    m_totalBytesLabel = new Plasma::Label(this);
    m_dirCountLabel = new Plasma::Label(this);
    m_fileCountLabel = new Plasma::Label(this);
    m_eta = new Plasma::Label(this);
    m_details = new Plasma::Label(this);

    m_totalBytesLabel->setVisible(false);
    m_dirCountLabel->setVisible(false);
    m_fileCountLabel->setVisible(false);

    m_fromNameLabel->setAlignment(Qt::AlignLeft);
    m_fromLabel->setAlignment(Qt::AlignLeft);
    m_toNameLabel->setAlignment(Qt::AlignLeft);
    m_toLabel->setAlignment(Qt::AlignLeft);
    m_totalBytesLabel->setAlignment(Qt::AlignRight);
    m_dirCountLabel->setAlignment(Qt::AlignRight);
    m_fileCountLabel->setAlignment(Qt::AlignRight);
    m_eta->setAlignment(Qt::AlignRight);
    m_details->setAlignment(Qt::AlignLeft);

    m_fromLabel->nativeWidget()->setWordWrap(false);
    m_toLabel->nativeWidget()->setWordWrap(false);
    m_dirCountLabel->nativeWidget()->setWordWrap(false);
    m_fileCountLabel->nativeWidget()->setWordWrap(false);
    m_totalBytesLabel->nativeWidget()->setWordWrap(false);
    m_eta->nativeWidget()->setWordWrap(false);

    m_layout = new QGraphicsGridLayout(this);
    m_layout->addItem(m_fromNameLabel, 0, 0);
    m_layout->addItem(m_fromLabel, 0, 1);
    m_layout->addItem(m_toNameLabel, 1, 0);
    m_layout->addItem(m_toLabel, 1, 1);
    m_layout->addItem(m_details, 2, 0);
    m_layout->addItem(m_eta, 2, 1);
    m_layout->addItem(m_meter, 3, 1);

    setMinimumWidth(350);

    if (m_job) {
        m_details->setText("<a href=\"more\">" + i18n("more") + "</a>");

        connect(m_job, SIGNAL(changed(SystemTray::Job*)), this, SLOT(updateJob()));
        connect(m_job, SIGNAL(destroyed(SystemTray::Job*)), this, SLOT(destroy()));
        connect(m_details, SIGNAL(linkActivated(const QString &)),
                this, SLOT(detailsLinkClicked(const QString &)));

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
        //TODO: make configurable:
        m_extenderItem->setAutoExpireDelay(15000);
        updateJob();
        m_meter->setValue(100);
    }
}

void JobWidget::updateJob()
{
    m_meter->setValue(m_job->percentage());

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
        item->setTitle(m_job->message());
        m_eta->setText(i18n("%1 (%2 remaining)", m_job->speed(),
                             KGlobal::locale()->prettyFormatDuration(m_job->eta())));
    } else if (m_job->state() == SystemTray::Job::Suspended) {
        item->setTitle(
            i18nc("%1 is the name of the job, can be things like Copying, deleting, moving",
                  "(paused) %1", m_job->message()));
        m_eta->setText(i18n("paused"));
    } else {
        item->setTitle(
            i18nc("%1 is the name of the job, can be things like Copying, deleting, moving",
                  "(finished) %1", m_job->message()));
        item->showCloseButton();
        m_details->hide();
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
        item->action("stop")->setVisible(m_job->isKillable() &&
                                         m_job->state() != SystemTray::Job::Stopped);
    }

    QMap<QString, qlonglong> processed = m_job->processedAmounts();
    QMap<QString, qlonglong> totals = m_job->totalAmounts();

    qlonglong dirs = totals.value("dirs");
    if (dirs > 1) {
        m_dirCountLabel->setText(i18np("%2 / 1 folder", "%2 / %1 folders", dirs, processed["dirs"]));
    }

    qlonglong files = totals.value("files");
    if (files > 1) {
        m_fileCountLabel->setText(i18np("%2 / 1 file", "%2 / %1 files", files, processed["files"]));
    }

    QString processedString = KGlobal::locale()->formatByteSize(processed["bytes"]);
    QString totalsString = KGlobal::locale()->formatByteSize(totals["bytes"]);
    m_totalBytesLabel->setText(QString("%1 / %2").arg(processedString, totalsString));

    item->setIcon(m_job->applicationIconName());
}

void JobWidget::resizeEvent(QGraphicsSceneResizeEvent *event)
{
    Q_UNUSED(event)
    updateLabels();
}

void JobWidget::updateLabels()
{
    Plasma::Theme *theme = Plasma::Theme::defaultTheme();
    QFont font = theme->font(Plasma::Theme::DefaultFont);
    QFontMetricsF fm(font);
    if (!labelName0.isEmpty()) {
        m_fromNameLabel->setText(QString("%1: ").arg(labelName0));
    }
    if (label0.startsWith("file://")) {
        label0 = KUrl(label0).toLocalFile();
    }

    m_fromLabel->setText(fm.elidedText(label0, Qt::ElideMiddle, m_fromLabel->size().width()));

    if (!labelName1.isEmpty()) {
        m_toNameLabel->setText(QString("%1: ").arg(labelName1));
    }
    if (label1.startsWith("file://")) {
        label1 = KUrl(label1).toLocalFile();
    }

    m_toLabel->setText(fm.elidedText(label1, Qt::ElideMiddle, m_toLabel->size().width()));
}

void JobWidget::detailsLinkClicked(const QString &link)
{
    if (link == "more") {
        m_details->setText("<a href=\"less\">" + i18n("less") + "</a>");
        m_totalBytesLabel->setVisible(true);
        m_dirCountLabel->setVisible(true);
        m_fileCountLabel->setVisible(true);
        m_layout->addItem(m_totalBytesLabel, 4, 1);
        m_layout->addItem(m_fileCountLabel, 5, 1);
        m_layout->addItem(m_dirCountLabel, 6, 1);
        m_extenderItem->setCollapsed(m_extenderItem->isCollapsed());
    } else {
        m_details->setText("<a href=\"more\">" + i18n("more") + "</a>");
        m_totalBytesLabel->setVisible(false);
        m_dirCountLabel->setVisible(false);
        m_fileCountLabel->setVisible(false);
        for (int i = 0; i < 3; i++) {
            m_layout->removeAt(m_layout->count() - 1);
        }
        m_layout->updateGeometry();
        m_extenderItem->setCollapsed(m_extenderItem->isCollapsed());
    }
}

#include "jobwidget.moc"

