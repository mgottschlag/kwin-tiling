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

#include <QTimer>

#include <KDebug>
#include <KService>
#include <KServiceTypeTrader>
#include <KStandardDirs>

namespace TaskManager
{


class TaskItem::Private
{
public:
    Private()
        : startupTask(0)
    {
    }

    QWeakPointer<Task> task;
    StartupPtr startupTask;
};


TaskItem::TaskItem(QObject *parent, TaskPtr task)
    : AbstractGroupableItem(parent),
      d(new Private)
{
    setTaskPointer(task);
}


TaskItem::TaskItem(QObject *parent, StartupPtr task)
    : AbstractGroupableItem(parent),
      d(new Private)
{
    d->startupTask = task;
    connect(task.data(), SIGNAL(changed(::TaskManager::TaskChanges)), this, SIGNAL(changed(::TaskManager::TaskChanges)));
    connect(task.data(), SIGNAL(destroyed(QObject*)), this, SLOT(taskDestroyed())); //this item isn't useful anymore if the Task was closed
}

TaskItem::~TaskItem()
{
    emit destroyed(this);
    //kDebug();
  /*  if (parentGroup()){
        parentGroup()->remove(this);
    }*/
    delete d;
}

void TaskItem::taskDestroyed()
{
    d->startupTask = 0;
    d->task.clear();
    // FIXME: due to a bug in Qt 4.x, the event loop reference count is incorrect
    // when going through x11EventFilter .. :/ so we have to singleShot the deleteLater
    QTimer::singleShot(0, this, SLOT(deleteLater()));
}

void TaskItem::setTaskPointer(TaskPtr task)
{
    if (d->startupTask) {
        disconnect(d->startupTask.data(), 0, this, 0);
        d->startupTask = 0;
    }

    if (d->task.data() != task.data()) {
        if (d->task) {
            disconnect(d->task.data(), 0, this, 0);
        }

        d->task = task.data();

        if (task) {
            connect(task.data(), SIGNAL(changed(::TaskManager::TaskChanges)), this, SIGNAL(changed(::TaskManager::TaskChanges)));
            connect(task.data(), SIGNAL(destroyed(QObject*)), this, SLOT(taskDestroyed()));
            emit gotTaskPointer();
        }
    }

    if (!d->task) {
        deleteLater();
    }
}

TaskPtr TaskItem::task() const
{
    if (!d->task) {
        return TaskPtr(0);
    }
    return TaskPtr(d->task.data());
}

StartupPtr TaskItem::startup() const
{
    /*
    if (d->startupTask.isNull()) {
        kDebug() << "pointer is Null";
    }
    */
    return d->startupTask;
}

bool TaskItem::isStartupItem() const
{
    return !d->startupTask.isNull();
}

WindowList TaskItem::winIds() const
{
    if (!d->task) {
        kDebug() << "no winId: probably startup task";
        return WindowList();
    }
    WindowList list;
    list << d->task.data()->window();
    return list;
}

QIcon TaskItem::icon() const
{
    if (d->task) {
        return d->task.data()->icon();
    }

    if (d->startupTask) {
        return d->startupTask->icon();
    }

    return QIcon();
}

QString TaskItem::name() const
{
    if (d->task) {
        return d->task.data()->visibleName();
    }

    if (d->startupTask) {
        return d->startupTask->text();
    }

    return QString();
}

ItemType TaskItem::itemType() const
{
    return TaskItemType;
}

bool TaskItem::isGroupItem() const
{
    return false;
}

void TaskItem::setShaded(bool state)
{
    if (!d->task) {
        return;
    }
    d->task.data()->setShaded(state);
}

void TaskItem::toggleShaded()
{
    if (!d->task) {
        return;
    }
    d->task.data()->toggleShaded();
}

bool TaskItem::isShaded() const
{
    if (!d->task) {
        return false;
    }
    return d->task.data()->isShaded();
}

void TaskItem::toDesktop(int desk)
{
    if (!d->task) {
        return;
    }
    d->task.data()->toDesktop(desk);
}

bool TaskItem::isOnCurrentDesktop() const
{
    return d->task && d->task.data()->isOnCurrentDesktop();
}

bool TaskItem::isOnAllDesktops() const
{
    return d->task && d->task.data()->isOnAllDesktops();
}

int TaskItem::desktop() const
{
    if (!d->task) {
        return 0;
    }
    return d->task.data()->desktop();
}

void TaskItem::setMaximized(bool state)
{
    if (!d->task) {
        return;
    }
    d->task.data()->setMaximized(state);
}

void TaskItem::toggleMaximized()
{
    if (!d->task) {
        return;
    }
    d->task.data()->toggleMaximized();
}

bool TaskItem::isMaximized() const
{
    return d->task && d->task.data()->isMaximized();
}

void TaskItem::setMinimized(bool state)
{
    if (!d->task) {
        return;
    }
    d->task.data()->setIconified(state);
}

void TaskItem::toggleMinimized()
{
    if (!d->task) {
        return;
    }
    d->task.data()->toggleIconified();
}

bool TaskItem::isMinimized() const
{
    if (!d->task) {
        return false;
    }
    return d->task.data()->isMinimized();
}

void TaskItem::setFullScreen(bool state)
{
    if (!d->task) {
        return;
    }
    d->task.data()->setFullScreen(state);
}

void TaskItem::toggleFullScreen()
{
    if (!d->task) {
        return;
    }
    d->task.data()->toggleFullScreen();
}

bool TaskItem::isFullScreen() const
{
    if (!d->task) {
        return false;
    }
    return d->task.data()->isFullScreen();
}

void TaskItem::setKeptBelowOthers(bool state)
{
    if (!d->task) {
        return;
    }
    d->task.data()->setKeptBelowOthers(state);
}

void TaskItem::toggleKeptBelowOthers()
{
    if (!d->task) {
        return;
    }
    d->task.data()->toggleKeptBelowOthers();
}

bool TaskItem::isKeptBelowOthers() const
{
    if (!d->task) {
        return false;
    }
    return d->task.data()->isKeptBelowOthers();
}

void TaskItem::setAlwaysOnTop(bool state)
{
    if (!d->task) {
        return;
    }
    d->task.data()->setAlwaysOnTop(state);
}

void TaskItem::toggleAlwaysOnTop()
{
    if (!d->task) {
        return;
    }
    d->task.data()->toggleAlwaysOnTop();
}

bool TaskItem::isAlwaysOnTop() const
{
    if (!d->task) {
        return false;
    }
    return d->task.data()->isAlwaysOnTop();
}

bool TaskItem::isActionSupported(NET::Action action) const
{
    return d->task && d->task.data()->info().actionSupported(action);
}

void TaskItem::addMimeData(QMimeData *mimeData) const
{
    if (!d->task) {
        return;
    }

    d->task.data()->addMimeData(mimeData);
}

KUrl TaskItem::launcherUrl() const
{
    if (!d->task) {
        return KUrl();
    }

    // Search for applications which are executable and case-insensitively match the windowclass of the task and
    // See http://techbase.kde.org/Development/Tutorials/Services/Traders#The_KTrader_Query_Language
    QString query = QString("exist Exec and ('%1' =~ Name)").arg(d->task.data()->classClass());
    KService::List services = KServiceTypeTrader::self()->query("Application", query);
    if (!services.empty()) {
        return KUrl::fromPath((services[0]->entryPath()));
    } else {
        // No desktop-file was found, so try to find at least the executable
        // usually it's the lower cased window class class, but if that fails let's trust it
        QString path = KStandardDirs::findExe(d->task.data()->classClass().toLower());
        if (path.isEmpty()) {
            path = KStandardDirs::findExe(d->task.data()->classClass());
        }

        if (!path.isEmpty()) {
            return KUrl::fromPath(path);
        }
    }

    return KUrl();
}

void TaskItem::close()
{
    if (!d->task) {
        return;
    }
    d->task.data()->close();
}

bool TaskItem::isActive() const
{
    if (!d->task) {
        return false;
    }
    return d->task.data()->isActive();
}

bool TaskItem::demandsAttention() const
{
    if (!d->task) {
        return false;
    }
    return d->task.data()->demandsAttention();
}

} // TaskManager namespace

#include "taskitem.moc"
