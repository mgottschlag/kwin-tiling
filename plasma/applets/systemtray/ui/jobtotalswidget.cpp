/***************************************************************************
 *   Copyright 2009 by Rob Scheepmaker <r.scheepmaker@student.utwente.nl>  *
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

#include "jobtotalswidget.h"
#include "../core/job.h"

#include <Plasma/ExtenderItem>
#include <Plasma/Meter>

static const int UPDATE_TIMER_INTERVAL = 200;

JobTotalsWidget::JobTotalsWidget(SystemTray::Job *job, Plasma::ExtenderItem *parent)
    : Meter(parent),
      m_extenderItem(parent),
      m_job(job),
      m_updateTimerId(0)
{
    Q_ASSERT(m_extenderItem);

    setSvg("widgets/bar_meter_horizontal");
    setMeterType(Plasma::Meter::BarMeterHorizontal);
    setMaximumHeight(16);
    setMinimumWidth(350);
    setMaximum(100);
    setValue(0);

    if (m_job) {
        connect(m_job, SIGNAL(changed(SystemTray::Job*)),
                this, SLOT(scheduleJobUpdate()));

        updateJob();
    }
}

JobTotalsWidget::~JobTotalsWidget()
{
}

void JobTotalsWidget::scheduleJobUpdate()
{
    if (!m_updateTimerId) {
        m_updateTimerId = startTimer(UPDATE_TIMER_INTERVAL);
    }
}

void JobTotalsWidget::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == m_updateTimerId) {
        killTimer(m_updateTimerId);
        m_updateTimerId = 0;
        updateJob();
    } else {
        Meter::timerEvent(event);
    }
}

void JobTotalsWidget::updateJob()
{
    setValue(m_job->percentage());

    m_extenderItem->setTitle(m_job->message());
    m_extenderItem->setIcon(m_job->applicationIconName());
}

#include "jobtotalswidget.moc"

