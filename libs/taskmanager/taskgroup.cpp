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

// Own
#include "taskgroup.h"
#include "taskmanager.h"
#include "taskitem.h"
#include "groupmanager.h"
//#include "abstractsortingstrategy.h"

// Qt
#include <QColor>

// KDE
#include <KDebug>
#include <KIcon>
#include <ksharedptr.h>



namespace TaskManager
{


class TaskGroup::Private
{
public:
    Private()
    {
    }

    ItemList m_members;
    QString m_groupName;
    QColor m_groupColor;
    QIcon m_groupIcon;
    bool m_aboutToDie;
    GroupManager *groupingStrategy;
};

TaskGroup::TaskGroup(GroupManager *parent,QString name, QColor color)
:   AbstractGroupableItem(parent),
    d(new Private)
{
    d->groupingStrategy = parent;
    d->m_groupName = name;
    d->m_groupColor = color;
    d->m_groupIcon = KIcon("xorg");

    kDebug() << "Group Created: Name: " << d->m_groupName << "Color: " << d->m_groupColor;
}

TaskGroup::TaskGroup(GroupManager *parent)
:   AbstractGroupableItem(parent),
    d(new Private)
{
    d->groupingStrategy = parent;
    d->m_groupName = "default";
    d->m_groupColor = Qt::red;
    d->m_groupIcon = KIcon("xorg");

    kDebug() << "Group Created: Name: " << d->m_groupName << "Color: " << d->m_groupColor;
}


TaskGroup::~TaskGroup()
{
    kDebug() << name();
    delete d;
}
/*
void TaskGroup::closeGroup()
{
    foreach(AbstractPtr item, d->m_members) { //delete all existing tasks
        if (item->isGroupItem()) { 
            (dynamic_cast<GroupPtr>(item))->clear();//FIXME is this reasonable or should they only be removed?
        }
        remove(item);
    }
    deleteLater();
    kDebug() << "Not implemented";
}*/

void TaskGroup::itemMoved(AbstractPtr item)
{
    emit itemChanged(item);
}

void TaskGroup::add(AbstractPtr item)
{
    if (!item->isGroupItem()) {
        if ((dynamic_cast<TaskItem*>(item))->taskPointer()) {
            kDebug() << "Add item" << (dynamic_cast<TaskItem*>(item))->taskPointer()->visibleName();
        }
        kDebug() << " to Group " << name();
    }

    if (d->m_members.contains(item)) {
        kDebug() << "already in this group";
        return;
    }

    if (item->parentGroup()) {
        item->parentGroup()->remove(item);
    }

    d->m_members.append(item);
    item->addedToGroup(this);
    /*if (d->groupingStrategy->taskSorter()) {
        d->groupingStrategy->taskSorter()->handleItem(item);
    }*/
    connect(item, SIGNAL(changed()), this, SIGNAL(changed()));
    //For debug
   /* foreach (AbstractGroupableItem *item, d->m_members) {
        if (item->isGroupItem()) {
            kDebug() << (dynamic_cast<TaskGroup*>(item))->name();
        } else {
            kDebug() << (dynamic_cast<TaskItem*>(item))->taskPointer()->visibleName();
        }
    }*/
    emit itemAdded(item);
}



void TaskGroup::remove(AbstractPtr item)
{
    Q_ASSERT(item);

    if (item->isGroupItem()) {
        kDebug() << "Remove group" << (dynamic_cast<TaskGroup*>(item))->name();
    } else if ((dynamic_cast<TaskItem*>(item))->taskPointer()) {
        kDebug() << "Remove item" << (dynamic_cast<TaskItem*>(item))->taskPointer()->visibleName();
    }
    kDebug() << "from Group: " << name();

   /* kDebug() << "GroupMembers: ";
    foreach (AbstractGroupableItem *item, members()) {
        if (item->isGroupItem()) {
            kDebug() << (dynamic_cast<TaskGroup*>(item))->name();
        } else {
            kDebug() << (dynamic_cast<TaskItem*>(item))->taskPointer()->visibleName();
        }
    }*/

    if (!d->m_members.contains(item)) {
        kDebug() << "couldn't find item";
        return;
    }
    disconnect(item, 0, this, 0);

    d->m_members.removeAll(item);
    item->removedFromGroup();
    /*if(d->m_members.isEmpty()){
        kDebug() << "empty";
        emit empty(this);
    }*/
    emit itemRemoved(item);
}

void TaskGroup::clear()
{
   // kDebug() << "size " << d->m_members.size();
    foreach(AbstractGroupableItem *item, d->m_members) {
    //    kDebug();
        Q_ASSERT(item);
        if (item->isGroupItem()) { 
            (dynamic_cast<GroupPtr>(item))->clear();
        }
        remove(item);
    }
    if (!d->m_members.isEmpty()) {
        kDebug() << "clear doesn't work";
    }
}

ItemList &TaskGroup::members() const
{
    return d->m_members;
}



void TaskGroup::setColor(const QColor &color)
{
    d->m_groupColor = color;
    emit changed();
}

QColor TaskGroup::color() const
{
    return d->m_groupColor;
}

QString TaskGroup::name() const
{
    return d->m_groupName;
}

void TaskGroup::setName(const QString &newName)
{
    d->m_groupName = newName;
    emit changed();
}

QIcon TaskGroup::icon() const
{
    return d->m_groupIcon;
}

void TaskGroup::setIcon(const QIcon &newIcon)
{
    d->m_groupIcon = newIcon;
    emit changed();
}



bool TaskGroup::isRootGroup()
{
    if (!parentGroup()) {
        return true;
    }
    return false;
}

/** only true if item is in this group */
bool TaskGroup::hasDirectMember(AbstractPtr item)
{
    return d->m_members.contains(item);
}

/** true if item is in this or any sub group */
bool TaskGroup::hasMember(AbstractPtr item)
{
    kDebug();
    TaskGroup *group = item->parentGroup();
    while (group) {
        if (group == this) {
            return true;
        }
        group = group->parentGroup();
    }
    return false;
}

/** Returns Direct Member group if the passed item is in a subgroup */
AbstractPtr TaskGroup::directMember(AbstractPtr item)
{
    AbstractPtr tempItem = item;
    while (tempItem) {
        if (d->m_members.contains(item)) {
            return item;
        }
        tempItem = tempItem->parentGroup();
    }
    kDebug() << "item not found";
    return AbstractPtr();
}

void TaskGroup::setShaded(bool state)
{
    foreach (AbstractPtr item, members()) {
        item->setShaded(state);
    }
}

void TaskGroup::toggleShaded()
{
    setShaded(!isShaded());
}

bool TaskGroup::isShaded()
{
    foreach (AbstractPtr item, members()) {
        if (!item->isShaded()) {
            return false;
        }
    }
    return true;
}

void TaskGroup::toDesktop(int desk)
{
    foreach (AbstractPtr item, members()) {
        item->toDesktop(desk);
    }
    emit movedToDesktop(desk);
}

bool TaskGroup::isOnCurrentDesktop()
{
    foreach (AbstractPtr item, members()) {
        if (!item->isOnCurrentDesktop()) {
            return false;
        }
    }
    return true;
}

bool TaskGroup::isOnAllDesktops()
{
    foreach (AbstractPtr item, members()) {
        if (!item->isOnAllDesktops()) {
            return false;
        }
    }
    return true;
}

//return 0 if tasks are on different desktops or on all dektops
int TaskGroup::desktop()
{
    int desk = members().first()->desktop();
    foreach (AbstractPtr item, members()) {
        if (item->desktop() != desk) {
            return 0;
        }
        desk = item->desktop();
    }
    return desk;
}

void TaskGroup::setMaximized(bool state)
{
    foreach (AbstractPtr item, members()) {
        item->setMaximized(state);
    }
}


void TaskGroup::toggleMaximized()
{
    setMaximized(!isMaximized());
}

bool TaskGroup::isMaximized()
{
    foreach (AbstractPtr item, members()) {
        if (!item->isMaximized()) {
            return false;
        }
    }
    return true;
}

void TaskGroup::setMinimized(bool state)
{
    foreach (AbstractPtr item, members()) {
        item->setMinimized(state);
    }
}

void TaskGroup::toggleMinimized()
{
    setMinimized(!isMinimized());
}

bool TaskGroup::isMinimized()
{
    foreach (AbstractPtr item, members()) {
        if (!item->isMinimized()) {
            return false;
        }
    }
    return true;
}

void TaskGroup::setFullScreen(bool state)
{
    foreach (AbstractPtr item, members()) {
        item->setFullScreen(state);
    }
}

void TaskGroup::toggleFullScreen()
{
    setFullScreen(!isFullScreen());
}

bool TaskGroup::isFullScreen()
{
    foreach (AbstractPtr item, members()) {
        if (!item->isFullScreen()) {
            return false;
        }
    }
    return true;
}

void TaskGroup::setKeptBelowOthers(bool state)
{
    foreach (AbstractPtr item, members()) {
        item->setKeptBelowOthers(state);
    }
}

void TaskGroup::toggleKeptBelowOthers()
{
    setKeptBelowOthers(!isKeptBelowOthers());
}

bool TaskGroup::isKeptBelowOthers()
{
    foreach (AbstractPtr item, members()) {
        if (!item->isKeptBelowOthers()) {
            return false;
        }
    }
    return true;
}

void TaskGroup::setAlwaysOnTop(bool state)
{
    foreach (AbstractPtr item, members()) {
        item->setAlwaysOnTop(state);
    }
}

void TaskGroup::toggleAlwaysOnTop()
{
    setAlwaysOnTop(!isAlwaysOnTop());
}

bool TaskGroup::isAlwaysOnTop()
{
    foreach (AbstractPtr item, members()) {
        if (!item->isAlwaysOnTop()) {
            return false;
        }
    }
    return true;
}


bool TaskGroup::isActionSupported(NET::Action action)
{
    if (KWindowSystem::allowedActionsSupported()) {
        foreach (AbstractPtr item, members()) {
            if (!item->isActionSupported(action)) {
                return false;
            }
        }
        return true;
    }
    return false;
}

void TaskGroup::close()
{
    foreach (AbstractPtr item, members()) {
        item->close();
    }
}

bool TaskGroup::isActive()
{
    foreach (AbstractPtr item, members()) {
        if (item->isActive()) {
            return true;
        }
    }

    return false;
}

bool TaskGroup::demandsAttention()
{
    foreach (AbstractPtr item, members()) {
        if (item->demandsAttention()) {
            return true;
        }
    }

    return false;
}

} // TaskManager namespace

#include "taskgroup.moc"
