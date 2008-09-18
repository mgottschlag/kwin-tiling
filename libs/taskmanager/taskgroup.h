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

#ifndef TASKGROUP_H
#define TASKGROUP_H

#include <QIcon>

#include "abstractgroupableitem.h"
#include <taskmanager/taskmanager_export.h>

namespace TaskManager
{
class GroupManager;

/**
 * TaskGroup, a container for tasks and subgroups
 */
class TASKMANAGER_EXPORT TaskGroup : public AbstractGroupableItem
{
    Q_OBJECT
public:
    TaskGroup(GroupManager *parent, QString name, QColor color);
    TaskGroup(GroupManager *parent);
    ~TaskGroup();
    
    ItemList &members() const;

    QIcon icon() const;
    void setIcon(const QIcon &icon);

    QColor color() const;
    void setColor(const QColor &color); 

    QString name() const;
    void setName(const QString &newName);

    virtual bool isGroupItem() const{return true;};
    bool isRootGroup();

    /** only true if item is in this group */
    bool hasDirectMember(AbstractPtr item);
    /** only true if item is in this or any sub group */
    bool hasMember(AbstractPtr item);
    /** Returns Direct Member group if the passed item is in a subgroup */
    AbstractPtr directMember(AbstractPtr);
    /** This function is used by the sortingStrategy to inform the visualization that the position of an item has changed */
    void itemMoved(AbstractPtr);

public slots:
    /** the following are functions which perform the corresponding actions on all member tasks */
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

    bool actionSupported(NET::Action);

    /** close all members of this group */
    void close();

    /** returns true if at least one member is active */
    bool isActive();
    /** returns true if at least one member is demands attention */
    bool demandsAttention();

    /** add item to group */
    void add(AbstractPtr);

    /** remove item from group */
    void remove(AbstractPtr);

    /** Removes all tasks and groups from this group */
    void clear();

    /** remove this group, passes all members to grouping strategy*/
 //   void closeGroup();
    
signals:
    /** inform visualization about wat is added and removed */
    void itemAdded(const AbstractPtr item);
    void itemRemoved(const AbstractPtr item);
   // void empty(TaskGroup*);
    void itemChanged(AbstractPtr item);
    /** The group changed the desktop, is emitted in the toDesktop function */
    void movedToDesktop(int newDesk);

private:
    class Private;
    Private * const d;
};


} // TaskManager namespace

#endif
