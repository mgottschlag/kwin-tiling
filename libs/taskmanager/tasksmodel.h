/*****************************************************************

Copyright 2010 Aaron Seigo <aseigo@kde.org

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

*****************************************************************/

#ifndef TASKSMODEL_H
#define TASKSMODEL_H

#include <QAbstractItemModel>

#include "taskmanager_export.h"

namespace TaskManager
{

class GroupManager;
class TasksModelPrivate;

class TASKMANAGER_EXPORT TasksModel : public QAbstractItemModel
{
    Q_OBJECT
    Q_ENUMS(DisplayRoles)

public:
    enum DisplayRoles {
        IsStartup = Qt::UserRole + 1,
        OnAllDesktops = Qt::UserRole + 2,
        Desktop = Qt::UserRole + 3,
        Shaded = Qt::UserRole + 4,
        Maximized = Qt::UserRole + 5,
        Minimized = Qt::UserRole + 6,
        FullScreen = Qt::UserRole + 7,
        BelowOthers = Qt::UserRole + 8,
        AlwaysOnTop = Qt::UserRole + 9,
        Active = Qt::UserRole + 10,
        DemandsAttention = Qt::UserRole + 11,
        LauncherUrl = Qt::UserRole + 12
    };

    explicit TasksModel(GroupManager *groupManager, QObject *parent = 0);
    ~TasksModel();

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &index) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;

private:
    friend class TasksModelPrivate;
    TasksModelPrivate * const d;

    Q_PRIVATE_SLOT(d, void populateModel())
    Q_PRIVATE_SLOT(d, void itemAboutToBeAdded(AbstractGroupableItem *, int))
    Q_PRIVATE_SLOT(d, void itemAdded(AbstractGroupableItem *))
    Q_PRIVATE_SLOT(d, void itemAboutToBeRemoved(AbstractGroupableItem *))
    Q_PRIVATE_SLOT(d, void itemRemoved(AbstractGroupableItem *))
    Q_PRIVATE_SLOT(d, void itemAboutToMove(AbstractGroupableItem *, int, int))
    Q_PRIVATE_SLOT(d, void itemMoved(AbstractGroupableItem *))
    Q_PRIVATE_SLOT(d, void itemChanged(::TaskManager::TaskChanges))
};

} // namespace TaskManager

#endif

