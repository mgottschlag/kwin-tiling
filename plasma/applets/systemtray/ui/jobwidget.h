/***************************************************************************
 *   Copyright 2008 by Rob Scheepmaker <r.scheepmaker@student.utwente.nl   *
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

#ifndef JOBWIDGET_H
#define JOBWIDGET_H

#include "../core/job.h"

#include <KJob>

#include <QGraphicsWidget>

#include <Plasma/Service>
#include <Plasma/ExtenderItem>
#include <plasma/dataengine.h>

namespace Plasma
{
    class ExtenderItem;
    class Label;
    class Meter;
} // namespace Plasma

namespace SystemTray
{
    class Job;
}

class JobWidget : public QGraphicsWidget
{
    Q_OBJECT

    public:
        explicit JobWidget(SystemTray::Job *job, Plasma::ExtenderItem *parent);
        ~JobWidget();

    public Q_SLOTS:
        void destroy();
        void updateJob();

    protected:
        void resizeEvent(QGraphicsSceneResizeEvent *event);

    private:
        void updateLabels();

        Plasma::ExtenderItem *m_extenderItem;
        SystemTray::Job *m_job;

        Plasma::Meter *m_meter;
        Plasma::Label *m_fromNameLabel;
        Plasma::Label *m_fromLabel;
        Plasma::Label *m_toNameLabel;
        Plasma::Label *m_toLabel;
        Plasma::Label *m_speedLabel;
        Plasma::Label *m_processedLabel;
        Plasma::Label *m_totalBytesLabel;

        QString labelName0;
        QString labelName1;
        QString label0;
        QString label1;
};

#endif
