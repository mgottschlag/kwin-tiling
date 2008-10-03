/***************************************************************************
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

#ifndef TASKITEM_H
#define TASKITEM_H

#include <taskmanager/abstractgroupableitem.h>
#include <taskmanager/startup.h>
#include <taskmanager/task.h>
#include <taskmanager/taskmanager_export.h>

#include <QtGui/QIcon>

namespace TaskManager
{


/**
 * Wrapper class so we dont have to use the Task class directly and the TaskPtr remains guarded
 */
class TASKMANAGER_EXPORT TaskItem : public AbstractGroupableItem
{
    Q_OBJECT
public:
    /** Creates a taskitem for a task*/
    TaskItem(QObject *parent, TaskPtr item);
    /** Creates a taskitem for a startuptask*/
    TaskItem(QObject *parent, StartupPtr item);
    ~TaskItem();
    /** Sets the taskpointer after the startup pointer */
    void setTaskPointer(TaskPtr);
    /** Returns the shared pointer to the  Task */
    TaskPtr taskPointer() const;

    StartupPtr startupPointer() const;
    bool isGroupItem() const{return false;};

    QIcon icon() const;
    QString name() const;

Q_SIGNALS::
    /** Indicates that the startup task now is a normal task */
    void gotTaskPointer();

public Q_SLOTS::

    void toDesktop(int);
    bool isOnCurrentDesktop();
    bool isOnAllDesktops();
    int desktop();

    void setShaded(bool);
    void toggleShaded();
    bool isShaded();

    void setMaximized(bool);
    void toggleMaximized();
    bool isMaximized();

    void setMinimized(bool);
    void toggleMinimized();
    bool isMinimized();

    void setFullScreen(bool);
    void toggleFullScreen();
    bool isFullScreen();

    void setKeptBelowOthers(bool);
    void toggleKeptBelowOthers();
    bool isKeptBelowOthers();

    void setAlwaysOnTop(bool);
    void toggleAlwaysOnTop();
    bool isAlwaysOnTop();

    bool isActive();
    bool demandsAttention();

    bool isActionSupported(NET::Action);

    void close();

private:
    class Private;
    Private * const d;
};

} // TaskManager namespace

#endif
