/*****************************************************************

Copyright (c) 2008 Christian Mollekopf <chrigi_1@hotmail.com>

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

#ifndef TASKACTIONS_P_H
#define TASKACTIONS_P_H

namespace TaskManager
{

/** Maximize a window or all windows in a group*/
class MaximizeActionImpl : public QAction
{
    Q_OBJECT
public:
    MaximizeActionImpl(QObject *parent, AbstractPtr task);
};


/** Minimize a window or all windows in a group*/
class MinimizeActionImpl : public QAction
{
    Q_OBJECT
public:
    MinimizeActionImpl(QObject *parent, AbstractPtr task);
};

/** Move window to current desktop*/
class ToCurrentDesktopActionImpl : public QAction
{
    Q_OBJECT
public:
    ToCurrentDesktopActionImpl(QObject *parent, AbstractPtr task);
private slots:
    void slotToCurrentDesktop();
private:
    AbstractPtr m_task;
};

/** Shade a window or all windows in a group*/
class ShadeActionImpl : public QAction
{
    Q_OBJECT
public:
    ShadeActionImpl(QObject *parent, AbstractPtr task);
};

/** Resize a window or all windows in a group*/
class ResizeActionImpl : public QAction
{
    Q_OBJECT
public:
    ResizeActionImpl(QObject *parent, TaskItem* task);
};

/** Move a window or all windows in a group*/
class MoveActionImpl : public QAction
{
    Q_OBJECT
public:
    MoveActionImpl(QObject *parent, TaskItem* task);
};

/** Shade a window or all windows in a group*/
class CloseActionImpl : public QAction
{
    Q_OBJECT
public:
    CloseActionImpl(QObject *parent, AbstractPtr task);
};

/** Send a Task to a specific Desktop*/
class ToDesktopActionImpl : public QAction
{
    Q_OBJECT
public:
    ToDesktopActionImpl(QObject *parent, AbstractPtr task, int desktop);
private slots:
    void slotToDesktop();
private:
    int m_desktop;
    AbstractPtr m_task;
};


/** Set a window or all windows in a group to FullScreen*/
class ViewFullscreenActionImpl : public QAction
{
    Q_OBJECT
public:
    ViewFullscreenActionImpl(QObject *parent, AbstractPtr task);
};

/** Keep a Window or all windows in a group above the rest */
class KeepAboveActionImpl : public QAction
{
    Q_OBJECT
public:
    KeepAboveActionImpl(QObject *parent, AbstractPtr task);
};

/** Keep a Window or all windows in a group below the rest*/
class KeepBelowActionImpl : public QAction
{
    Q_OBJECT
public:
    KeepBelowActionImpl(QObject *parent, AbstractPtr task);
};


/** Leave current Group*/
class LeaveGroupActionImpl : public QAction
{
    Q_OBJECT
public:
    LeaveGroupActionImpl(QObject *parent, AbstractPtr task, GroupManager*);

private slots:
    void leaveGroup();

private:
    AbstractPtr abstractItem;
    GroupManager *groupingStrategy;
};

/** Remove Group
class RemoveGroupActionImpl : public QAction
{
    Q_OBJECT
public:
    RemoveGroupActionImpl(QObject *parent, AbstractPtr task);
};
*/

} // TaskManager namespace


#endif
