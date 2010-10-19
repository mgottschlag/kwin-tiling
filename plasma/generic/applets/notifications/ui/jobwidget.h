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


#include <QGraphicsWidget>
#include <QGraphicsGridLayout>

#include <Plasma/Service>
#include <Plasma/ExtenderItem>
#include <plasma/dataengine.h>
#include <Plasma/PushButton>

namespace Plasma
{
    class ExtenderItem;
    class PushButton;
    class Label;
    class Meter;
    class IconWidget;
    class SignalPlotter;
} // namespace Plasma

class Job;


class JobWidget : public QGraphicsWidget
{
    Q_OBJECT

    public:
        explicit JobWidget(Job *job, Plasma::ExtenderItem *parent);
        ~JobWidget();

        void poppedUp(bool shown);

        Job *job() const;

    protected:
        void resizeEvent(QGraphicsSceneResizeEvent *event);
        void timerEvent(QTimerEvent *event);
        void showEvent(QShowEvent *event);
        void hideEvent(QHideEvent *event);

    private Q_SLOTS:
        void detailsClicked();
        void destroyExtenderItem();
        void scheduleUpdateJob();
        void updateJobState();

    private:
        void updateLabels();
        void updateJob();

        Plasma::ExtenderItem *m_extenderItem;
        QWeakPointer<Job>m_job;

        Plasma::Meter *m_meter;
        Plasma::Label *m_fromNameLabel;
        Plasma::Label *m_fromLabel;
        Plasma::Label *m_toNameLabel;
        Plasma::Label *m_toLabel;
        Plasma::Label *m_totalBytesLabel;
        Plasma::Label *m_dirCountLabel;
        Plasma::Label *m_fileCountLabel;
        Plasma::Label *m_eta;
        Plasma::IconWidget *m_details;
        Plasma::SignalPlotter *m_plotter;

        QGraphicsGridLayout *m_layout;

        QString labelName0;
        QString labelName1;
        QString label0;
        QString label1;

        int m_updateTimerId;

        bool m_extenderItemDestroyed;
};

#endif
