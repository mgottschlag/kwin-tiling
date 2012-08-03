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


#include <QFont>
#include <QAction>
#include <QGraphicsGridLayout>
#include <QLabel>

#include <KIconLoader>

#include <Plasma/DataEngine>
#include <Plasma/Extender>
#include <Plasma/ExtenderItem>
#include <Plasma/Label>
#include <Plasma/Meter>
#include <Plasma/PopupApplet>
#include <Plasma/PushButton>
#include <Plasma/Service>
#include <Plasma/SignalPlotter>
#include <Plasma/IconWidget>
#include <Plasma/Theme>
#include <Plasma/ToolTipManager>

static const int UPDATE_INTERVAL = 200;

JobWidget::JobWidget(Job *job, Plasma::ExtenderItem *parent)
    : QGraphicsWidget(parent),
      m_extenderItem(parent),
      m_job(job),
      m_updateTimerId(0),
      m_extenderItemDestroyed(false)
{
    Q_ASSERT(m_extenderItem);

    m_meter = new Plasma::Meter(this);
    m_meter->setSvg("widgets/bar_meter_horizontal");
    m_meter->setMeterType(Plasma::Meter::BarMeterHorizontal);
    m_meter->setMaximumHeight(16);
    m_meter->setMaximum(100);
    m_meter->setValue(0);

    m_plotter = new Plasma::SignalPlotter(this);
    m_plotter->setUseAutoRange(true);
    m_plotter->setShowVerticalLines(false);
    m_plotter->setUnit(i18n("KiB/s"));
    m_plotter->addPlot(Plasma::Theme::defaultTheme()->color(Plasma::Theme::HighlightColor));

    m_fromNameLabel = new Plasma::Label(this);
    m_fromLabel = new Plasma::Label(this);
    m_toNameLabel = new Plasma::Label(this);
    m_toLabel = new Plasma::Label(this);
    m_totalBytesLabel = new Plasma::Label(this);
    m_dirCountLabel = new Plasma::Label(this);
    m_fileCountLabel = new Plasma::Label(this);
    m_eta = new Plasma::Label(this);
    m_details = new Plasma::IconWidget(this);
    m_details->setSvg("widgets/action-overlays", "add-normal");
    m_details->setMaximumSize(KIconLoader::SizeSmallMedium, KIconLoader::SizeSmallMedium);
    m_details->setMinimumSize(KIconLoader::SizeSmallMedium, KIconLoader::SizeSmallMedium);

    m_totalBytesLabel->setVisible(false);
    m_dirCountLabel->setVisible(false);
    m_fileCountLabel->setVisible(false);
    m_plotter->setVisible(false);

    m_fromNameLabel->setAlignment(Qt::AlignRight);
    m_fromLabel->setAlignment(Qt::AlignLeft);
    m_toNameLabel->setAlignment(Qt::AlignRight);
    m_toLabel->setAlignment(Qt::AlignLeft);
    m_totalBytesLabel->setAlignment(Qt::AlignRight);
    m_dirCountLabel->setAlignment(Qt::AlignRight);
    m_fileCountLabel->setAlignment(Qt::AlignRight);
    m_eta->setAlignment(Qt::AlignRight);

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
    m_layout->addItem(m_eta, 2, 1);
    m_layout->addItem(m_details, 3, 0, Qt::AlignVCenter|Qt::AlignRight);
    m_layout->addItem(m_meter, 3, 1, Qt::AlignCenter);

    setMinimumWidth(350);

    if (m_job.data()) {
        m_details->setToolTip(i18n("More"));
        m_details->setSvg("widgets/action-overlays", "add-normal");

        connect(m_job.data(), SIGNAL(stateChanged(Job*)), this, SLOT(updateJobState()));
        connect(m_job.data(), SIGNAL(destroyed(Job*)), this, SLOT(destroyExtenderItem()));
        connect(m_details, SIGNAL(clicked()), this, SLOT(detailsClicked()));

        //the suspend action
        QAction *suspendAction = new QAction(m_extenderItem);
        suspendAction->setIcon(KIcon("media-playback-pause"));
        suspendAction->setEnabled(true);
        suspendAction->setVisible(false);
        suspendAction->setToolTip(i18n("Pause job"));
        m_extenderItem->addAction("suspend", suspendAction);
        connect(suspendAction, SIGNAL(triggered()), m_job.data(), SLOT(suspend()));

        //the resume action
        QAction *resumeAction = new QAction(m_extenderItem);
        resumeAction->setIcon(KIcon("media-playback-start"));
        resumeAction->setEnabled(true);
        resumeAction->setVisible(false);
        resumeAction->setToolTip(i18n("Resume job"));
        m_extenderItem->addAction("resume", resumeAction);
        connect(resumeAction, SIGNAL(triggered()), m_job.data(), SLOT(resume()));

        //the friendly stop action
        QAction *stopAction = new QAction(m_extenderItem);
        stopAction->setIcon(KIcon("media-playback-stop"));
        stopAction->setEnabled(true);
        stopAction->setVisible(true);
        stopAction->setToolTip(i18n("Cancel job"));
        m_extenderItem->addAction("stop", stopAction);
        connect(stopAction, SIGNAL(triggered()), m_job.data(), SLOT(stop()));

        updateJob();
        updateJobState();  // make sure to set the title
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

void JobWidget::destroyExtenderItem()
{
    m_extenderItem->destroy();
    m_extenderItemDestroyed = true;
}

void JobWidget::scheduleUpdateJob()
{
    if (m_extenderItemDestroyed) {
        return;
    }

    if (!m_updateTimerId) {
        m_updateTimerId = startTimer(UPDATE_INTERVAL);
    }
}

void JobWidget::updateJobState()
{
    if (m_extenderItemDestroyed && m_job.data()) {
        return;
    }

    //show the current status in the title.
    if (!m_job.data()->error().isEmpty()) {
        m_extenderItem->setTitle(m_job.data()->error());
    } else if (m_job.data()->state() == Job::Running) {
        m_extenderItem->setTitle(m_job.data()->message());
        if (m_job.data()->eta()) {
            m_eta->setText(i18n("%1 (%2 remaining)", m_job.data()->speed(),
                                 KGlobal::locale()->prettyFormatDuration(m_job.data()->eta())));
        } else {
            m_eta->setText(QString());
        }
    } else if (m_job.data()->state() == Job::Suspended) {
        m_extenderItem->setTitle(
            i18nc("%1 is the name of the job, can be things like Copying, deleting, moving",
                  "%1 [Paused]", m_job.data()->message()));
        m_eta->setText(i18n("Paused"));
    } else {
        m_extenderItem->setTitle(
            i18nc("%1 is the name of the job, can be things like Copying, deleting, moving",
                  "%1 [Finished]", m_job.data()->message()));
        m_extenderItem->showCloseButton();
    }
}

void JobWidget::updateJob()
{
    if (m_extenderItemDestroyed || !m_job.data()) {
        return;
    }

    m_meter->setValue(m_job.data()->percentage());

    //Update the ETA and job speed (only if running)
    if (m_job.data()->state() == Job::Running) {
        if (m_job.data()->eta()) {
            m_eta->setText(i18n("%1 (%2 remaining)", m_job.data()->speed(),
                                 KGlobal::locale()->prettyFormatDuration(m_job.data()->eta())));
        } else {
            m_eta->setText(QString());
        }
    }

    if (m_job.data()->labels().count() > 0) {
        labelName0 = m_job.data()->labels().value(0).first;
        label0 = m_job.data()->labels().value(0).second;
    }
    if (m_job.data()->labels().count() > 1) {
        labelName1 = m_job.data()->labels().value(1).first;
        label1 = m_job.data()->labels().value(1).second;
    }

    //TODO: can we write this at some later point?
    KConfigGroup cg = m_extenderItem->config();
    cg.writeEntry("labelName0", labelName0);
    cg.writeEntry("label0", label0);
    cg.writeEntry("labelName1", labelName1);
    cg.writeEntry("label1", label1);

    updateLabels();

    //set the correct actions to visible.
    if (m_extenderItem->action("suspend")) {
        m_extenderItem->action("suspend")->setVisible(m_job.data()->isSuspendable() &&
                                            m_job.data()->state() == Job::Running);
    }

    if (m_extenderItem->action("resume")) {
        m_extenderItem->action("resume")->setVisible(m_job.data()->isSuspendable() &&
                                           m_job.data()->state() == Job::Suspended);
    }

    if (m_extenderItem->action("stop")) {
        m_extenderItem->action("stop")->setVisible(m_job.data()->isKillable() &&
                                         m_job.data()->state() != Job::Stopped);
    }

    QMap<QString, qlonglong> processed = m_job.data()->processedAmounts();
    QMap<QString, qlonglong> totals = m_job.data()->totalAmounts();

    qlonglong dirs = totals.value("dirs");
    if (dirs > 1) {
        m_dirCountLabel->setText(i18np("%2 / 1 folder", "%2 / %1 folders", dirs, processed["dirs"]));
        m_dirCountLabel->setMaximumHeight(INT_MAX);
    } else {
        m_dirCountLabel->setMaximumHeight(0);
    }

    qlonglong files = totals.value("files");
    if (files > 1) {
        m_fileCountLabel->setText(i18np("%2 / 1 file", "%2 / %1 files", files, processed["files"]));
        m_fileCountLabel->setMaximumHeight(INT_MAX);
    } else {
        m_fileCountLabel->setMaximumHeight(0);
    }

    QList<double> sample;
    sample << m_job.data()->numericSpeed()/1000;
    m_plotter->addSample(sample);

    qlonglong total = totals["bytes"];
    if (total > 0) {
        QString processedString = KGlobal::locale()->formatByteSize(processed["bytes"]);
        QString totalsString = KGlobal::locale()->formatByteSize(total);
        m_totalBytesLabel->setText(QString("%1 / %2").arg(processedString, totalsString));
    } else {
        m_details->hide();
        m_totalBytesLabel->hide();
    }

    if (m_totalBytesLabel->text().isEmpty() &&
        m_dirCountLabel->text().isEmpty() &&
        m_fileCountLabel->text().isEmpty()) {
        m_details->hide();
    } else {
        m_details->show();
    }

    m_extenderItem->setIcon(m_job.data()->applicationIconName());
}

void JobWidget::showEvent(QShowEvent *)
{
    if (!m_job.data()) {
        return;
    }

    Plasma::PopupApplet *applet = qobject_cast<Plasma::PopupApplet *>(m_extenderItem->extender()->applet());
    if (applet && applet->isPopupShowing()) {
        updateJob();
        disconnect(m_job.data(), SIGNAL(changed(Job*)), this, SLOT(scheduleUpdateJob()));
        connect(m_job.data(), SIGNAL(changed(Job*)), this, SLOT(scheduleUpdateJob()));
        return;
    }
}

void JobWidget::hideEvent(QHideEvent *)
{
    if (!m_job.data()) {
        return;
    }

    disconnect(m_job.data(), SIGNAL(changed(Job*)), this, SLOT(scheduleUpdateJob()));
}

void JobWidget::poppedUp(bool shown)
{
    if (!m_job.data()) {
        return;
    }

    disconnect(m_job.data(), SIGNAL(changed(Job*)), this, SLOT(scheduleUpdateJob()));

    if (shown && isVisible()) {
        updateJob();
        connect(m_job.data(), SIGNAL(changed(Job*)), this, SLOT(scheduleUpdateJob()));
        return;
    }
}

Job *JobWidget::job() const
{
    return m_job.data();
}

void JobWidget::resizeEvent(QGraphicsSceneResizeEvent *event)
{
    Q_UNUSED(event)
    updateLabels();
}

void JobWidget::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == m_updateTimerId) {
        killTimer(m_updateTimerId);
        m_updateTimerId = 0;
        updateJob();
    }
}

void JobWidget::updateLabels()
{
    QFontMetricsF fm = m_fromLabel->nativeWidget()->fontMetrics();
    if (!labelName0.isEmpty()) {
        m_fromNameLabel->setText(QString("%1: ").arg(labelName0));
    }
    if (label0.startsWith(QLatin1String("file://"))) {
        label0 = KUrl(label0).toLocalFile();
    }

    const QString shortLabel0(fm.elidedText(label0, Qt::ElideMiddle, m_fromLabel->size().width()));
    m_fromLabel->setText(shortLabel0);


    Plasma::ToolTipContent data;

    if (label0.length() > shortLabel0.length()) {
        data.setSubText(label0);
        Plasma::ToolTipManager::self()->setContent(m_fromLabel, data);
    }

    if (!labelName1.isEmpty()) {
        m_toNameLabel->setText(QString("%1: ").arg(labelName1));
    }
    if (label1.startsWith(QLatin1String("file://"))) {
        label1 = KUrl(label1).toLocalFile();
    }

    const QString shortLabel1(fm.elidedText(label1, Qt::ElideMiddle, m_toLabel->size().width()));
    m_toLabel->setText(shortLabel1);

    if (label1.length() > shortLabel1.length()) {
        data.setSubText(label1);
        Plasma::ToolTipManager::self()->setContent(m_toLabel, data);
    }
}

void JobWidget::detailsClicked()
{
    if (!m_totalBytesLabel->isVisible()) {
        m_details->setToolTip(i18n("Less"));
        m_details->setSvg("widgets/action-overlays", "remove-normal");
        m_totalBytesLabel->setVisible(true);
        m_dirCountLabel->setVisible(true);
        m_fileCountLabel->setVisible(true);
        m_plotter->setVisible(true);
        m_layout->addItem(m_totalBytesLabel, 4, 1);
        m_layout->addItem(m_fileCountLabel, 5, 1);
        m_layout->addItem(m_dirCountLabel, 6, 1);
        m_layout->addItem(m_plotter, 7, 1);
        m_extenderItem->setCollapsed(m_extenderItem->isCollapsed());
    } else {
        m_details->setToolTip(i18n("More"));
        m_details->setSvg("widgets/action-overlays", "add-normal");
        m_totalBytesLabel->setVisible(false);
        m_dirCountLabel->setVisible(false);
        m_fileCountLabel->setVisible(false);
        m_plotter->setVisible(false);
        for (int i = 0; i < 4; i++) {
            m_layout->removeAt(m_layout->count() - 1);
        }
        m_layout->updateGeometry();
        m_extenderItem->setCollapsed(m_extenderItem->isCollapsed());
    }
}

#include "jobwidget.moc"

