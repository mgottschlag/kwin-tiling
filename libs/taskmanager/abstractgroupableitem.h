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

#ifndef ABSTRACTGROUPABLEITEM_H
#define ABSTRACTGROUPABLEITEM_H

#include <QObject>
#include <QSet>

#include <kdemacros.h> //for the KDE_EXPORT macro

#include <kwindowsystem.h>

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
class KDE_EXPORT AbstractGroupableItem : public QObject
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
