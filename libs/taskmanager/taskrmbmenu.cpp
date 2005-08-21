/*****************************************************************

Copyright (c) 2001 Matthias Elter <elter@kde.org>
Copyright (c) 2001 John Firebaugh <jfirebaugh@kde.org>

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

#include <assert.h>

#include <kiconloader.h>
#include <klocale.h>

#include "taskmanager.h"

#if defined(HAVE_XCOMPOSITE) && \
    defined(HAVE_XRENDER) && \
    defined(HAVE_XFIXES)
#include <fixx11h.h>
#endif

#include "taskrmbmenu.h"
#include "taskrmbmenu.moc"

TaskRMBMenu::TaskRMBMenu(const Task::List& theTasks, bool show, QWidget *parent, const char *name)
	: QMenu( parent )
	, tasks( theTasks )
	, showAll( show )
{
    setName(name);
    assert(tasks.count() > 0);
    if (tasks.count() == 1)
    {
        fillMenu(tasks.first());
    }
    else
    {
        fillMenu();
    }
}

TaskRMBMenu::TaskRMBMenu(Task::TaskPtr task, bool show, QWidget *parent, const char *name)
	: QMenu( parent )
	, showAll( show )
{
	setName(name);      
	fillMenu(task);
}

void TaskRMBMenu::fillMenu(Task::TaskPtr t)
{
    int id;
    setCheckable(true);

    insertItem(i18n("Ad&vanced"), makeAdvancedMenu(t));
    bool checkActions = KWin::allowedActionsSupported();

    if (TaskManager::self()->numberOfDesktops() > 1)
    {
        id = insertItem(i18n("To &Desktop"), makeDesktopsMenu(t));

        if (showAll)
        {
            id = insertItem(i18n("&To Current Desktop"),
                            t, SLOT(toCurrentDesktop()));
            setItemEnabled( id, !t->isOnCurrentDesktop() );
        }

        if (checkActions)
        {
            setItemEnabled(id, t->info().actionSupported(NET::ActionChangeDesktop));
        }
    }

    id = insertItem(SmallIconSet("move"), i18n("&Move"), t, SLOT(move()));
    setItemEnabled(id, !checkActions || t->info().actionSupported(NET::ActionMove));

    id = insertItem(i18n("Re&size"), t, SLOT(resize()));
    setItemEnabled(id, !checkActions || t->info().actionSupported(NET::ActionResize));

    id = insertItem(i18n("Mi&nimize"), t, SLOT(toggleIconified()));
    setItemChecked(id, t->isIconified());
    setItemEnabled(id, !checkActions || t->info().actionSupported(NET::ActionMinimize));

    id = insertItem(i18n("Ma&ximize"), t, SLOT(toggleMaximized()));
    setItemChecked(id, t->isMaximized());
    setItemEnabled(id, !checkActions || t->info().actionSupported(NET::ActionMax));

    id = insertItem(i18n("&Shade"), t, SLOT(toggleShaded()));
    setItemChecked(id, t->isShaded());
    setItemEnabled(id, !checkActions || t->info().actionSupported(NET::ActionShade));

    insertSeparator();

    id = insertItem(SmallIcon("fileclose"), i18n("&Close"), t, SLOT(close()));
    setItemEnabled(id, !checkActions || t->info().actionSupported(NET::ActionClose));
}

void TaskRMBMenu::fillMenu()
{
	int id;
	setCheckable( true );

    Task::List::iterator itEnd = tasks.end();
    for (Task::List::iterator it = tasks.begin(); it != itEnd; ++it)
    {
		Task::TaskPtr t = (*it);

		id = insertItem( QIcon( t->pixmap() ),
				 t->visibleNameWithState(),
		                 new TaskRMBMenu(t, this) );
		setItemChecked( id, t->isActive() );
		connectItem( id, t, SLOT( activateRaiseOrIconify() ) );
	}

	insertSeparator();

    bool enable = false;

    if (TaskManager::self()->numberOfDesktops() > 1)
    {
        id = insertItem(i18n("All to &Desktop"), makeDesktopsMenu());

        id = insertItem(i18n("All &to Current Desktop"), this, SLOT(slotAllToCurrentDesktop()));
        Task::List::iterator itEnd = tasks.end();
        for (Task::List::iterator it = tasks.begin(); it != itEnd; ++it)
        {
            if (!(*it)->isOnCurrentDesktop())
            {
                enable = true;
                break;
            }
        }
        setItemEnabled(id, enable);
    }

    enable = false;

	id = insertItem( i18n( "Mi&nimize All" ), this, SLOT( slotMinimizeAll() ) );
    itEnd = tasks.end();
    for (Task::List::iterator it = tasks.begin(); it != itEnd; ++it)
    {
		if( !(*it)->isIconified() ) {
			enable = true;
			break;
		}
	}
	setItemEnabled( id, enable );

	enable = false;

	id = insertItem( i18n( "Ma&ximize All" ), this, SLOT( slotMaximizeAll() ) );
    itEnd = tasks.end();
    for (Task::List::iterator it = tasks.begin(); it != itEnd; ++it)
    {
        if( !(*it)->isMaximized() ) {
			enable = true;
			break;
		}
	}
	setItemEnabled( id, enable );

	enable = false;

	id = insertItem( i18n( "&Restore All" ), this, SLOT( slotRestoreAll() ) );
    itEnd = tasks.end();
    for (Task::List::iterator it = tasks.begin(); it != itEnd; ++it)
    {
		if( (*it)->isIconified() || (*it)->isMaximized() ) {
			enable = true;
			break;
		}
	}
	setItemEnabled( id, enable );

	insertSeparator();

	enable = false;

	insertItem( SmallIcon( "remove" ), i18n( "&Close All" ), this, SLOT( slotCloseAll() ) );
}

QMenu* TaskRMBMenu::makeAdvancedMenu(Task::TaskPtr t)
{
    int id;
    QMenu* menu = new QMenu(this);

    menu->setCheckable(true);

    id = menu->insertItem(SmallIconSet("up"),
                          i18n("Keep &Above Others"),
                          t, SLOT(toggleAlwaysOnTop()));
    menu->setItemChecked(id, t->isAlwaysOnTop());

    id = menu->insertItem(SmallIconSet("down"),
                          i18n("Keep &Below Others"),
                          t, SLOT(toggleKeptBelowOthers()));
    menu->setItemChecked(id, t->isKeptBelowOthers());

    id = menu->insertItem(SmallIconSet("window_fullscreen"),
                          i18n("&Fullscreen"),
                          t, SLOT(toggleFullScreen()));
    menu->setItemChecked(id, t->isFullScreen());

    if (KWin::allowedActionsSupported())
    {
        menu->setItemEnabled(id, t->info().actionSupported(NET::ActionFullScreen));
    }

    return menu;
}

QMenu* TaskRMBMenu::makeDesktopsMenu(Task::TaskPtr t)
{
	QMenu* m = new QMenu( this );
	m->setCheckable( true );

	int id = m->insertItem( i18n("&All Desktops"), t, SLOT( toDesktop(int) ) );
	m->setItemParameter( id, 0 ); // 0 means all desktops
	m->setItemChecked( id, t->isOnAllDesktops() );

	m->insertSeparator();

	for (int i = 1; i <= TaskManager::self()->numberOfDesktops(); i++) {
		QString name = QString("&%1 %2").arg(i).arg(TaskManager::self()->desktopName(i).replace('&', "&&"));
		id = m->insertItem( name, t, SLOT( toDesktop(int) ) );
		m->setItemParameter( id, i );
		m->setItemChecked( id, !t->isOnAllDesktops() && t->desktop() == i );
	}

	return m;
}

QMenu* TaskRMBMenu::makeDesktopsMenu()
{
	QMenu* m = new QMenu( this );
	m->setCheckable( true );

	int id = m->insertItem( i18n("&All Desktops"), this, SLOT( slotAllToDesktop(int) ) );
	m->setItemParameter( id, 0 ); // 0 means all desktops

	m->insertSeparator();

	for (int i = 1; i <= TaskManager::self()->numberOfDesktops(); i++) {
		QString name = QString("&%1 %2").arg(i).arg(TaskManager::self()->desktopName(i).replace('&', "&&"));
		id = m->insertItem( name, this, SLOT( slotAllToDesktop(int) ) );
		m->setItemParameter( id, i );
	}

	return m;
}

void TaskRMBMenu::slotMinimizeAll()
{
    Task::List::iterator itEnd = tasks.end();
    for (Task::List::iterator it = tasks.begin(); it != itEnd; ++it)
    {
        (*it)->setIconified(true);
    }
}

void TaskRMBMenu::slotMaximizeAll()
{
    Task::List::iterator itEnd = tasks.end();
    for (Task::List::iterator it = tasks.begin(); it != itEnd; ++it)
    {
        (*it)->setMaximized(true);
    }
}

void TaskRMBMenu::slotRestoreAll()
{
    Task::List::iterator itEnd = tasks.end();
    for (Task::List::iterator it = tasks.begin(); it != itEnd; ++it)
    {
        (*it)->restore();
    }
}

void TaskRMBMenu::slotShadeAll()
{
    Task::List::iterator itEnd = tasks.end();
    for (Task::List::iterator it = tasks.begin(); it != itEnd; ++it)
    {
        (*it)->setShaded( !(*it)->isShaded() );
    }
}

void TaskRMBMenu::slotCloseAll()
{
    Task::List::iterator itEnd = tasks.end();
    for (Task::List::iterator it = tasks.begin(); it != itEnd; ++it)
    {
        (*it)->close();
    }
}

void TaskRMBMenu::slotAllToDesktop( int desktop )
{
    Task::List::iterator itEnd = tasks.end();
    for (Task::List::iterator it = tasks.begin(); it != itEnd; ++it)
    {
        (*it)->toDesktop( desktop );
    }
}

void TaskRMBMenu::slotAllToCurrentDesktop()
{
    Task::List::iterator itEnd = tasks.end();
    for (Task::List::iterator it = tasks.begin(); it != itEnd; ++it)
    {
        (*it)->toCurrentDesktop();
    }
}
