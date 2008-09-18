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

#ifndef PROGRAMGROUPINGSTRATEGY_H
#define PROGRAMGROUPINGSTRATEGY_H

#include "abstractgroupingstrategy.h"
#include "taskgroup.h"

namespace TaskManager
{

class GroupManager;

/**
 * Groups tasks of the same program
 */
class KDE_EXPORT ProgramGroupingStrategy: public AbstractGroupingStrategy
{
    Q_OBJECT
public:
    ProgramGroupingStrategy(GroupManager *groupManager);
    ~ProgramGroupingStrategy();

    void handleItem(AbstractPtr);
    GroupManager::TaskGroupingStrategy type() const;

    /** Returns list of actions that a task can do in this groupingStrategy
    *  fore example: start/stop group tasks of this program
    */
    QList <QAction*> *strategyActions(QObject *parent, AbstractGroupableItem *item);

    EditableGroupProperties editableGroupProperties(){return None;};

protected slots:
    /** Checks if the group is still necessary */
    void checkGroup();
private slots:
    void toggleGrouping();

private:
    bool programGrouping(TaskItem* taskItem, TaskGroup* groupItem);
    class Private;
    Private * const d;

};


}



#endif
