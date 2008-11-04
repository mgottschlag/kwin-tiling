/***************************************************************************
 *   Copyright (C) 2008 Marco Martin <notmart@gmail.com>                   *
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


#ifndef TASKSMENU_H
#define TASKSMENU_H


#include <taskmanager/taskactions.h>

namespace Plasma
{
    class Applet;
    class FrameSvg;
}

namespace TaskManager
{

/**
 * A Layout for the expanded group
 */
class TasksMenu : public GroupPopupMenu
{
    Q_OBJECT

public:
    TasksMenu(QWidget *parent, TaskGroup *group, GroupManager *groupManager, Plasma::Applet *applet);

    ~TasksMenu();

protected:
    void paintEvent(QPaintEvent *event);
    void resizeEvent(QResizeEvent *event);

    Plasma::FrameSvg *m_background;
    Plasma::FrameSvg *m_itemBackground;
    Plasma::Applet *m_applet;
};
}

#endif
