/*****************************************************************

Copyright 2008 Christian Mollekopf <robertknight@gmail.com>

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

#ifndef ABSTRACTGROUPABLEITEM_H
#define ABSTRACTGROUPABLEITEM_H

#include <QObject>
#include <QSet>

#include <kwindowsystem.h>

#include <taskmanager/taskmanager_export.h>

namespace TaskManager
{

class TaskGroup;
class AbstractGroupableItem;

typedef AbstractGroupableItem* AbstractPtr;
typedef TaskGroup* GroupPtr;

typedef QList<AbstractPtr> ItemList;
typedef QList<GroupPtr> GroupList;

/**
 * Abstract Class for an Item that is groupable
 * So groups can handle tasks and subgroups the same way
 */
class TASKMANAGER_EXPORT AbstractGroupableItem : public QObject
{
    Q_OBJECT
public:
    AbstractGroupableItem(QObject *parent);
    virtual ~AbstractGroupableItem();

    /**
    *   Returns the parent group of this item
    */
    GroupPtr parentGroup() const;

    /**
    *   Not only member of rootGroup
    */
    bool grouped() const;

    bool isGroupMember(const GroupPtr group);
    virtual bool isGroupItem() const = 0;

public slots:

    /** Functions that both, Tasks and Groups have */
    virtual void toDesktop(int) = 0;
    virtual bool isOnCurrentDesktop() = 0;
    virtual bool isOnAllDesktops() = 0;
    virtual int desktop() = 0;

    virtual void setShaded(bool) = 0;
    virtual void toggleShaded() = 0;
    virtual bool isShaded() = 0;

    virtual void setMaximized(bool) = 0;
    virtual void toggleMaximized() = 0;
    virtual bool isMaximized() = 0;

    virtual void setMinimized(bool) = 0;
    virtual void toggleMinimized() = 0;
    virtual bool isMinimized() = 0;

    virtual void setFullScreen(bool) = 0;
    virtual void toggleFullScreen() = 0;
    virtual bool isFullScreen() = 0;

    virtual void setKeptBelowOthers(bool) = 0;
    virtual void toggleKeptBelowOthers() = 0;
    virtual bool isKeptBelowOthers() = 0;

    virtual void setAlwaysOnTop(bool) = 0;
    virtual void toggleAlwaysOnTop() = 0;
    virtual bool isAlwaysOnTop() = 0;
    
    virtual bool actionSupported(NET::Action) = 0;
    
    virtual void close() = 0;

    virtual bool isActive() = 0;
    virtual bool demandsAttention() = 0;

    void removedFromGroup();
    void addedToGroup(const GroupPtr group);

signals:
    void changed();
    void destroyed(AbstractGroupableItem *);

private:
    class Private;
    Private * const d;
};

} // TaskManager namespace

#endif
