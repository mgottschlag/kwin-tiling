/***************************************************************************
 *   Copyright 2009 by Rob Scheepmaker <r.scheepmaker@student.utwente.nl   *
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

#ifndef JOBTOTALSWIDGET_H
#define JOBTOTALSWIDGET_H

#include "../core/job.h"


#include <QGraphicsWidget>

#include <plasma/widgets/meter.h>
#include <Plasma/Service>
#include <Plasma/ExtenderGroup>
#include <plasma/dataengine.h>

namespace Plasma
{
    class ExtenderItem;
    class Meter;
} // namespace Plasma


class Job;

class JobTotalsWidget : public Plasma::Meter
{
    Q_OBJECT

    public:
        explicit JobTotalsWidget(Job *job, QGraphicsWidget *parent);
        ~JobTotalsWidget();

    protected:
        void timerEvent(QTimerEvent *event);

    private Q_SLOTS:
        void scheduleJobUpdate();

    private:
        void updateJob();

        Plasma::ExtenderGroup *m_extenderGroup;
        Job *m_job;
        int m_updateTimerId;
};


#endif
