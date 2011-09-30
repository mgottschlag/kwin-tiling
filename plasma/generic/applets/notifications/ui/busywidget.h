/***************************************************************************
 *   Copyright (C) 2008 Rob Scheepmaker <r.scheepmaker@student.utwente.nl> *
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

#ifndef NOTIFICATIONBUSYWIDGET_H
#define NOTIFICATIONBUSYWIDGET_H

#include <KIcon>

#include <Plasma/BusyWidget>

class QStyleOptionGraphicsItem;
class QSequentialAnimationGroup;

namespace Plasma
{
    class Animation;
    class Extender;
    class PopupApplet;
    class Svg;
}


class Manager;

class BusyWidget : public Plasma::BusyWidget
{
    Q_OBJECT

public:
    enum State { Empty, Info, Running };

    BusyWidget(Plasma::PopupApplet *parent, const Manager *manager);
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);
    void setState(State state);
    void suppressToolTips(bool suppress);

public slots:
    void toolTipAboutToShow();

protected:
    void resizeEvent(QGraphicsSceneResizeEvent *event);

protected slots:
    void updateTask();

private:
    QString expanderElement() const;
    void getJobCounts(int &runningJobs, int &pausedJobs, int &completedJobs, int &jobSpeed);

    KIcon m_icon;
    State m_state;
    Plasma::Svg *m_svg;
    Plasma::PopupApplet *m_systray;
    const Manager *m_manager;
    Plasma::Animation *m_fadeInAnimation;
    Plasma::Animation *m_fadeOutAnimation;
    QSequentialAnimationGroup *m_fadeGroup;
    int m_total;
    bool m_suppressToolTips;
};


#endif
