/***************************************************************************
 *   taskarea.h                                                            *
 *                                                                         *
 *   Copyright (C) 2008 Jason Stubbs <jasonbstubbs@gmail.com>              *
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

#ifndef TASKAREA_H
#define TASKAREA_H

#include <QGraphicsWidget>

#include "../protocols/dbussystemtray/dbussystemtraytask.h"

#include <Plasma/Plasma>

namespace SystemTray
{

class Applet;
class Task;

class TaskArea : public QGraphicsWidget
{
    Q_OBJECT

public:
    TaskArea(SystemTray::Applet *parent);
    ~TaskArea();

    QGraphicsWidget *hiddenTasksWidget() const;
    void setHiddenTypes(const QStringList &hiddenTypes);
    QStringList hiddenTypes() const;
    void setAlwaysShownTypes(const QStringList &alwaysShownTypes);
    QStringList alwaysShownTypes() const;
    bool isHiddenType(const QString &typeId, bool always = true) const;
    void setShowFdoTasks(bool show);
    bool showFdoTasks() const;
    void syncTasks(const QList<SystemTray::Task*> &tasks);
    int leftEasement() const;
    int rightEasement() const;
    void setOrientation(Qt::Orientation);
    void setLocation(Plasma::Location location);

public slots:
    void addTask(SystemTray::Task *task);
    void removeTask(SystemTray::Task *task);
    void delayedAppletUpdate();
    void updateUnhideToolIcon();

signals:
    void sizeHintChanged(Qt::SizeHint which);
    void toggleHiddenItems();

private slots:
    void relayoutHiddenTasks();
    void adjustHiddenTasksWidget();

private:
    bool addWidgetForTask(SystemTray::Task *task);
    QGraphicsWidget* findWidget(Task *task);
    bool checkUnhideTool();
    void checkVisibility(Task *task);
    bool removeFromHiddenArea(SystemTray::Task *task);

    class Private;
    Private* const d;
};

}


#endif
