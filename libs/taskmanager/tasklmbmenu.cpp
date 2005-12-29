/*****************************************************************

Copyright (c) 2001 Matthias Elter <elter@kde.org>
Copyright (c) 2002 John Firebaugh <jfirebaugh@kde.org>

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

#include "tasklmbmenu.h"
#include "tasklmbmenu.moc"

#include <qpainter.h>
#include <qstyle.h>
#include <QMenuItem>
//Added by qt3to4:
#include <QDragLeaveEvent>
#include <QDragMoveEvent>
#include <QList>
#include <QDragEnterEvent>
#include <QMouseEvent>

#include <kdebug.h>
#include <kglobalsettings.h>

#include <kmenu.h>

#include "utils.h"

#if 0
TaskMenuItem::TaskMenuItem(const QString &text,
                           bool active, bool minimized, bool attention)
  : QCustomMenuItem(),
    m_text(text),
    m_isActive(active),
    m_isMinimized(minimized),
    m_demandsAttention(attention),
    m_attentionState(true)
{
}

TaskMenuItem::~TaskMenuItem()
{
}

void TaskMenuItem::paint(QPainter *p, const QColorGroup &cg,
                         bool highlighted, bool /*enabled*/,
                         int x, int y, int w, int h )
{
    if (m_isActive)
    {
        QFont font = p->font();
        font.setBold(true);
        p->setFont(font);
    }

    if (highlighted)
    {
        p->setPen(cg.highlightedText());
    }
    else if (m_isMinimized)
    {
        p->setPen(QPen(Plasma::blendColors(cg.background(), cg.text())));
    }
    else if (m_demandsAttention && !m_attentionState)
    {
        p->setPen(cg.mid());
    }

    p->drawText(x, y, w, h, Qt::AlignLeft|Qt::AlignVCenter|Qt::TextDontClip|Qt::TextShowMnemonic, m_text);
}

QSize TaskMenuItem::sizeHint()
{
    QFont font = QFont();
    if (m_isActive)
    {
        font.setBold(true);
    }
    return QFontMetrics(font).size(AlignAuto|AlignVCenter|DontClip|ShowPrefix,
                                   m_text);
}
#endif

/*****************************************************************************/

TaskLMBMenu::TaskLMBMenu(const Task::List& tasks, QWidget *parent, const char *name)
  : QMenu(parent),
    m_tasks(tasks),
    m_lastDragId(-1),
    m_attentionState(false)
{
    setName(name);
    fillMenu();

    setAcceptDrops(true); // Always enabled to activate task during drag&drop.

    m_dragSwitchTimer = new QTimer(this, "DragSwitchTimer");
    connect(m_dragSwitchTimer, SIGNAL(timeout()), SLOT(dragSwitch()));
}

void TaskLMBMenu::fillMenu()
{

    Task::List::iterator itEnd = m_tasks.end();
    for (Task::List::iterator it = m_tasks.begin(); it != itEnd; ++it)
    {
        Task::TaskPtr t = (*it);

        QString text = t->visibleName().replace("&", "&&");

        //### KDE4
/*        TaskMenuItem *menuItem = new TaskMenuItem(text,
                                                  t->isActive(),
                                                  t->isIconified(),
                                                  t->demandsAttention());*/
        //QAction* menuItem = 
        int id = insertItem(QIcon(t->pixmap()), text);
        connectItem(id, t.data(), SLOT(activateRaiseOrIconify()));
        setItemChecked(id, t->isActive());

        if (t->demandsAttention())
        {
            m_attentionState = true;
            m_attentionMap.append(actions().at(indexOf(id)));
        }
    }

    if (m_attentionState)
    {
        m_attentionTimer = new QTimer(this, "AttentionTimer");
        connect(m_attentionTimer, SIGNAL(timeout()), SLOT(attentionTimeout()));
        m_attentionTimer->start(500, true);
    }
}

void TaskLMBMenu::attentionTimeout()
{
    m_attentionState = !m_attentionState;

    //### KDE4
#if 0
    for (Q3ValueList<TaskMenuItem*>::const_iterator it = m_attentionMap.constBegin();
         it != m_attentionMap.constEnd();
         ++it)
    {
        (*it)->setAttentionState(m_attentionState);
    }
#endif

    update();

    m_attentionTimer->start(500, true);
}

void TaskLMBMenu::dragEnterEvent( QDragEnterEvent* e )
{
    // ignore task drags
    if (TaskDrag::canDecode(e->mimeData()))
    {
        return;
    }

    int id = static_cast<QMenuItem*>(actionAt(e->pos()))->id();

    if (id == -1)
    {
        m_dragSwitchTimer->stop();
        m_lastDragId = -1;
    }
    else if (id != m_lastDragId)
    {
        m_lastDragId = id;
        m_dragSwitchTimer->start(1000, true);
    }

    QMenu::dragEnterEvent( e );
}

void TaskLMBMenu::dragLeaveEvent( QDragLeaveEvent* e )
{
    m_dragSwitchTimer->stop();
    m_lastDragId = -1;

    QMenu::dragLeaveEvent(e);

    hide();
}

void TaskLMBMenu::dragMoveEvent( QDragMoveEvent* e )
{
    // ignore task drags
    if (TaskDrag::canDecode(e->mimeData()))
    {
        return;
    }

    int id = static_cast<QMenuItem*>(actionAt(e->pos()))->id();

    if (id == -1)
    {
        m_dragSwitchTimer->stop();
        m_lastDragId = -1;
    }
    else if (id != m_lastDragId)
    {
        m_lastDragId = id;
        m_dragSwitchTimer->start(1000, true);
    }

    QMenu::dragMoveEvent(e);
}

void TaskLMBMenu::dragSwitch()
{
    Task::TaskPtr t = m_tasks.at(indexOf(m_lastDragId));
    if (t)
    {
        t->activate();

        for (unsigned int i = 0; i < count(); ++i)
        {
            setItemChecked(idAt(i), false );
        }

        setItemChecked( m_lastDragId, true );
    }
}

void TaskLMBMenu::mousePressEvent( QMouseEvent* e )
{
    if (e->button() == Qt::LeftButton)
    {
        m_dragStartPos = e->pos();
    }
    else
    {
        m_dragStartPos = QPoint();
    }

    QMenu::mousePressEvent(e);
}

void TaskLMBMenu::mouseReleaseEvent(QMouseEvent* e)
{
    m_dragStartPos = QPoint();
    QMenu::mouseReleaseEvent(e);
}

void TaskLMBMenu::mouseMoveEvent(QMouseEvent* e)
{
    if (m_dragStartPos.isNull())
    {
        QMenu::mouseMoveEvent(e);
        return;
    }

    int delay = KGlobalSettings::dndEventDelay();
    QPoint newPos(e->pos());

    if ((m_dragStartPos - newPos).manhattanLength() > delay)
    {
        int index = actions().indexOf(actionAt(e->pos()));
        if (index != -1)
        {
            Task::TaskPtr task = m_tasks.at(index);
            if (task)
            {
                Task::List tasks;
                tasks.append(task);
                TaskDrag* drag = new TaskDrag(tasks, this);
                drag->setPixmap(task->pixmap());
                drag->start();
            }
        }
    }

    QMenu::mouseMoveEvent(e);
}

