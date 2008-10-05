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
#include "taskitem.h"
#include <KDebug>


namespace TaskManager
{


class TaskItem::Private
{
public:
    Private()
        :task(0),
        startupTask(0)
    {
    }

    TaskPtr task;
    StartupPtr startupTask;
};


TaskItem::TaskItem(QObject *parent,TaskPtr task)
:   AbstractGroupableItem(parent),
    d(new Private)
{
    d->task = task;
    connect(task.data(), SIGNAL(changed(::TaskManager::TaskChanges)), this, SIGNAL(changed()));
    connect(task.data(), SIGNAL(destroyed()), this, SLOT(deleteLater())); //this item isn't useful anymore if the Task was closed
}


TaskItem::TaskItem(QObject *parent, StartupPtr task)
:   AbstractGroupableItem(parent),
    d(new Private)
{
    d->startupTask = task;
    connect(task.data(), SIGNAL(changed(::TaskManager::TaskChanges)), this, SIGNAL(changed()));
    connect(task.data(), SIGNAL(destroyed()), this, SLOT(deleteLater())); //this item isn't useful anymore if the Task was closed
}

TaskItem::~TaskItem()
{
    //kDebug();
  /*  if (parentGroup()){
        parentGroup()->remove(this);
    }*/
    delete d;
}

void TaskItem::setTaskPointer(TaskPtr task)
{
    if (d->startupTask) {
        disconnect(d->startupTask.data(), 0, 0, 0);
        d->startupTask = 0;
    }
    d->task = task;
    connect(task.data(), SIGNAL(changed(::TaskManager::TaskChanges)), this, SIGNAL(changed()));
    connect(task.data(), SIGNAL(destroyed()), this, SLOT(deleteLater()));
    emit gotTaskPointer();
}

TaskPtr TaskItem::task() const
{
    if (d->task.isNull()) {
        kDebug() << "pointer is Null";
    }
    return d->task;
}

StartupPtr TaskItem::startupPointer() const
{
    if (d->startupTask.isNull()) {
        kDebug() << "pointer is Null";
    }
    return d->startupTask;
}

QIcon TaskItem::icon() const
{
    if (!d->task) {
        return QIcon();
    }
    return task()->icon();
}

QString TaskItem::name() const
{
    if (!d->task) {
        return QString();
    }
    return task()->visibleName();
}

void TaskItem::setShaded(bool state)
{
    if (!d->task) {
        return;
    }
    task()->setShaded(state);
}

void TaskItem::toggleShaded()
{
    if (!d->task) {
        return;
    }
    task()->toggleShaded();
}

bool TaskItem::isShaded()
{
    if (!d->task) {
        return false;
    }
    return task()->isShaded();
}

void TaskItem::toDesktop(int desk)
{
    if (!d->task) {
        return;
    }
    task()->toDesktop(desk);
}

bool TaskItem::isOnCurrentDesktop()
{
    if (!d->task) {
        return false;
    }
    return task()->isOnCurrentDesktop();
}

bool TaskItem::isOnAllDesktops()
{
    if (!d->task) {
        return false;
    }
    return task()->isOnAllDesktops();
}

int TaskItem::desktop()
{
    if (!d->task) {
        return 0;
    }
    return task()->desktop();
}

void TaskItem::setMaximized(bool state)
{
    if (!d->task) {
        return;
    }
    task()->setMaximized(state);
}

void TaskItem::toggleMaximized()
{
    if (!d->task) {
        return;
    }
    task()->toggleMaximized();
}

bool TaskItem::isMaximized()
{
    if (!d->task) {
        return false;
    }
    return task()->isMaximized();
}

void TaskItem::setMinimized(bool state)
{
    if (!d->task) {
        return;
    }
    task()->setIconified(state);
}

void TaskItem::toggleMinimized()
{
    if (!d->task) {
        return;
    }
    task()->toggleIconified();
}

bool TaskItem::isMinimized()
{
    if (!d->task) {
        return false;
    }
    return task()->isMinimized();
}

void TaskItem::setFullScreen(bool state)
{
    if (!d->task) {
        return;
    }
    task()->setFullScreen(state);
}

void TaskItem::toggleFullScreen()
{
    if (!d->task) {
        return;
    }
    task()->toggleFullScreen();
}

bool TaskItem::isFullScreen()
{
    if (!d->task) {
        return false;
    }
    return task()->isFullScreen();
}

void TaskItem::setKeptBelowOthers(bool state)
{
    if (!d->task) {
        return;
    }
    task()->setKeptBelowOthers(state);
}

void TaskItem::toggleKeptBelowOthers()
{
    if (!d->task) {
        return;
    }
    task()->toggleKeptBelowOthers();
}

bool TaskItem::isKeptBelowOthers()
{       
    if (!d->task) {
        return false;
    }
    return task()->isKeptBelowOthers();
}

void TaskItem::setAlwaysOnTop(bool state)
{
    if (!d->task) {
        return;
    }
    task()->setAlwaysOnTop(state);
}

void TaskItem::toggleAlwaysOnTop()
{
    if (!d->task) {
        return;
    }
    task()->toggleAlwaysOnTop();
}

bool TaskItem::isAlwaysOnTop()
{
    if (!d->task) {
        return false;
    }
    return task()->isAlwaysOnTop();
}

bool TaskItem::isActionSupported(NET::Action action)
{
    if (!d->task) {
        return false;
    }
    if (KWindowSystem::allowedActionsSupported()) {
       return (task()->info().actionSupported(action));
    }
    return false;
    //return (!KWindowSystem::allowedActionsSupported() || d->task->info().isActionSupported(action));
}

void TaskItem::close()
{
    if (!d->task) {
        return;
    }
    task()->close();
}

bool TaskItem::isActive()
{
    if (!d->task) {
        return false;
    }
    return task()->isActive();
}

bool TaskItem::demandsAttention()
{
    if (!d->task) {
        return false;
    }
    return task()->demandsAttention();
}

} // TaskManager namespace

#include "taskitem.moc"
