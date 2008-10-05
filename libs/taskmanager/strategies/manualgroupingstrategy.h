/*****************************************************************

Copyright 2008 Christian Mollekopf <chrigi_1@hotmail.com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

******************************************************************/

#ifndef MANUALGROUPINGSTRATEGY_H
#define MANUALGROUPINGSTRATEGY_H

#include "abstractgroupingstrategy.h"
#include "taskgroup.h"

namespace TaskManager
{

class ManualGroupingStrategy;
/**
 * TaskGroup, a container for tasks and subgroups
 */

class TaskGroupTemplate : public AbstractGroupableItem
{
    Q_OBJECT
public:
    TaskGroupTemplate(ManualGroupingStrategy *parent, TaskGroup *group);
    ~TaskGroupTemplate();

    TaskGroup *group();
    void setGroup(TaskGroup *);

    ItemList &members() const;
    QIcon icon() const;
    QColor color() const;
    QString name() const;

    bool isGroupItem() const { return true; };

    /** only true if item is in this group */
    bool hasDirectMember(AbstractPtr item);
    /** only true if item is in this or any sub group */
    bool hasMember(AbstractPtr item);
    /** Returns Direct Member group if the passed item is in a subgroup */
    AbstractPtr directMember(AbstractPtr);

    TaskGroupTemplate *findParentGroup(AbstractPtr item);

    TaskGroupTemplate *parentGroup();
    void setParentGroup(TaskGroupTemplate *);

Q_SIGNALS:
    /** Unprotects group so it can get closed
    */
    void unprotectGroup(TaskGroup *);
    /** used to inform the group that there is still a desktop with this group so it wont
    *	be closed when empty
    */
    void protectGroup(TaskGroup *);
    
    void destroyed(AbstractGroupableItem *);

private Q_SLOTS:
    void itemDestroyed(AbstractGroupableItem *);


public Q_SLOTS:
    /** Needed because we subclass AbstractGroupableItem */
    void toDesktop(int){};
    bool isOnCurrentDesktop(){return false;};
    bool isOnAllDesktops(){return false;};
    int desktop(){return 0;};

    void setShaded(bool){};
    void toggleShaded(){};
    bool isShaded(){return false;};

    void setMaximized(bool){};
    void toggleMaximized(){};
    bool isMaximized(){return false;};

    void setMinimized(bool){};
    void toggleMinimized(){};
    bool isMinimized(){return false;};

    void setFullScreen(bool){};
    void toggleFullScreen(){};
    bool isFullScreen(){return false;};

    void setKeptBelowOthers(bool){};
    void toggleKeptBelowOthers(){};
    bool isKeptBelowOthers(){return false;};

    void setAlwaysOnTop(bool){};
    void toggleAlwaysOnTop(){};
    bool isAlwaysOnTop(){return false;};

    bool isActionSupported(NET::Action){return false;};

    /** close all members of this group */
    void close(){};

    /** returns true if at least one member is active */
    bool isActive(){return false;};
    /** returns true if at least one member is demands attention */
    bool demandsAttention(){return false;};

    /** add item to group */
    void add(AbstractPtr);

    /** remove item from group */
    void remove(AbstractPtr);

    /** Removes all tasks and groups from this group */
    void clear();

    /** remove this group, passes all members to grouping strategy*/
    void closeGroup();


private:
    class Private;
    Private * const d;
};




class GroupManager;
/**
 * Remebers manually grouped tasks
 * To do this it keeps an exact copy of the rootGroup and all subgroups
 * for each desktop/screen 
 */
class ManualGroupingStrategy: public AbstractGroupingStrategy
{
    Q_OBJECT
public:
    ManualGroupingStrategy(GroupManager *groupingStrategy);
    ~ManualGroupingStrategy();

    /** looks up if this item has been grouped before and groups it accordingly.
    *otherwise the item goes to the rootGroup
    */
    void handleItem(AbstractPtr);
    /** Should be called if the user wants to manually add an item to a group */
    //bool addItemToGroup(AbstractGroupableItem*, TaskGroup*);
    /** Should be called if the user wants to group items manually */
    bool groupItems(ItemList items);

    /** Returns list of actions that a task can do in this groupingStrategy
    *  fore example: remove this Task from this group
    */
    QList<QAction*> strategyActions(QObject *parent, AbstractGroupableItem *item);

    EditableGroupProperties editableGroupProperties();

    void desktopChanged(int newDesktop);

private slots:

    /** Actions which the strategy offers*/
    /** sender item leaves group*/
    void leaveGroup();
    /** Removes all items from the sender group and adds to the parent Group*/
    void removeGroup();


    void groupChangedDesktop(int newDesk);
    /** Protects group from being closed, because the tasks in the group are just temporarily 
    *	not available (not on the desktop,...). Every TaskGroupTemplate calls this so the group is
    *	is only closed if it isn't present on any desktop.
    */
    void protectGroup(TaskGroup *group);
    /** Unprotects group so it can get closed
    */
    void unprotectGroup(TaskGroup *group);
    /** This function makes sure that if the rootGroup template already got deleted nobody tries to access it again*/
    void resetCurrentTemplate();

protected:
    void closeGroup(TaskGroup*);

private:
    bool manualGrouping(TaskItem* taskItem, TaskGroup* groupItem);

    /** Create a duplication of a group with all subgroups TaskItems arent duplicated */
    TaskGroupTemplate *createDuplication(TaskGroup *group);

    class Private;
    Private * const d;
};


}



#endif
