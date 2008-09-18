/*
    Copyright (C) 2007 Robert Knight <robertknight@gmail.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the
    Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .
 */

#ifndef ABSTRACTGROUPINGSTRATEGY_H
#define ABSTRACTGROUPINGSTRATEGY_H

#include "abstractgroupableitem.h"
#include "groupmanager.h"
#include "taskgroup.h"
#include <QObject>
#include <QMenu>



namespace TaskManager
{

/**
 * Base class for strategies which can be used to
 * automatically group tasks.
 */
class KDE_EXPORT AbstractGroupingStrategy : public QObject
{
    Q_OBJECT
public:
    AbstractGroupingStrategy(GroupManager *groupManager);
    virtual ~AbstractGroupingStrategy();

     /** Handles a new item */
    virtual void handleItem(AbstractPtr) = 0;

     /** Returns the strategy type */
    virtual GroupManager::TaskGroupingStrategy type() const = 0;

    /** DesktopChanges time to backup any needed data */
    virtual void desktopChanged(int newDesktop){};

    /** Returns list of actions that a task can do in this groupingStrategy
    *  If the visualisation supports grouping it has to show these actions.
    */
    virtual QList <QAction*> *strategyActions(QObject *parent, AbstractGroupableItem *item);

    enum EditableGroupProperties
    {
        None = 0,
        Name = 1,
        Color = 2,
        Icon =  4,
        Members = 8,
        All = 15
    };
    /** Returns which group properties are editable by the user and which are handled solely by the strategy. 
    * The visualization should create a configuration interface based on this.
    */
    virtual EditableGroupProperties editableGroupProperties() = 0;
  
    /** The following functions check if a property is editable and sets it on group*/
    virtual bool addItemToGroup(AbstractGroupableItem *, TaskGroup*);

    virtual bool setName(const QString &, TaskGroup*);
    /** Returns a List of unused Names*/
    virtual QList <QString> nameSuggestions(TaskGroup *);

    virtual bool setColor(const QColor &, TaskGroup*);
    /** Returns a list of unused colors*/
    virtual QList <QColor> colorSuggestions(TaskGroup *);

    virtual bool setIcon(const QIcon &, TaskGroup*);
    /** Returns a list of icons*/
    virtual QList <QIcon> iconSuggestions(TaskGroup *);

protected slots:
    /** Adds all group members to the parentgroup of group and removes the group */
    virtual void closeGroup(TaskGroup *group);

    /** Checks if the group is still necessary, removes group if empty*/
    virtual void checkGroup();

protected:
    /** Create a group with items and returns the newly created group */
    virtual TaskGroup* createGroup(ItemList items);

private:
    class Private;
    Private * const d;


    
};

} // TaskManager namespace
#endif
