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

#ifndef TASKITEM_H
#define TASKITEM_H

#include <taskmanager/abstractgroupableitem.h>
#include <taskmanager/startup.h>
#include <taskmanager/task.h>
#include <taskmanager/taskmanager_export.h>

#include <QtGui/QIcon>

namespace TaskManager
{


/**
 * Wrapper class so we do not have to use the Task class directly and the TaskPtr remains guarded
 */
class TASKMANAGER_EXPORT TaskItem : public AbstractGroupableItem
{
    Q_OBJECT
public:
    /** Creates a taskitem for a task*/
    TaskItem(QObject *parent, TaskPtr item);
    /** Creates a taskitem for a startuptask*/
    TaskItem(QObject *parent, StartupPtr item);
    ~TaskItem();
    /** Sets the taskpointer after the startup pointer */
    void setTaskPointer(TaskPtr);
    /** Returns a pointer to the  Task; may be NULL */
    Task *task() const;

    WindowList winIds() const;

    StartupPtr startup() const;
    ItemType itemType() const;
    /**
    * @deprecated: use itemType() instead
    **/
    KDE_DEPRECATED bool isGroupItem() const;

    QIcon icon() const;
    QString name() const;
    QString taskName() const;

    bool isStartupItem() const;
    bool isOnCurrentDesktop() const;
    bool isOnAllDesktops() const;
    int desktop() const;
    bool isShaded() const;
    bool isMaximized() const;
    bool isMinimized() const;
    bool isFullScreen() const;
    bool isKeptBelowOthers() const;
    bool isAlwaysOnTop() const;
    bool isActive() const;
    bool demandsAttention() const;
    bool isActionSupported(NET::Action) const;
    void addMimeData(QMimeData *mimeData) const;
    void setLauncherUrl(const KUrl &url);
    void setLauncherUrl(const AbstractGroupableItem *item);
    KUrl launcherUrl() const;
    void resetLauncherCheck();

public Q_SLOTS:
    void toDesktop(int);

    void setShaded(bool);
    void toggleShaded();

    void setMaximized(bool);
    void toggleMaximized();

    void setMinimized(bool);
    void toggleMinimized();

    void setFullScreen(bool);
    void toggleFullScreen();

    void setKeptBelowOthers(bool);
    void toggleKeptBelowOthers();

    void setAlwaysOnTop(bool);
    void toggleAlwaysOnTop();

    void close();
    void taskDestroyed();

Q_SIGNALS:
    /** Indicates that the startup task now is a normal task */
    void gotTaskPointer();

private:
    class Private;
    Private * const d;
};

} // TaskManager namespace

#endif
