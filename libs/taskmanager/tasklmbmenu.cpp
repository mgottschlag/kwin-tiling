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

#include <qregexp.h>

#include "tasklmbmenu.h"
#include "tasklmbmenu.moc"

TaskLMBMenu::TaskLMBMenu( TaskList* tasks, QWidget *parent, const char *name )
	: QPopupMenu( parent, name )
	, m_tasks( *tasks )
	, m_lastDragId( -1 )
{
	fillMenu( tasks );
	
	setAcceptDrops(true); // Always enabled to activate task during drag&drop.
	
	connect( &dragSwitchTimer, SIGNAL( timeout() ), SLOT( dragSwitch() ) );
}

void TaskLMBMenu::fillMenu( TaskList* tasks )
{
	setCheckable( true );

	for( QPtrListIterator<Task> it(*tasks); *it; ++it ) {
		Task* t = (*it);

		QString text = t->visibleNameWithState().replace(QRegExp("&"), "&&");

		int id = insertItem( QIconSet( t->pixmap() ), text,
				     t, SLOT( activateRaiseOrIconify() ) );
		setItemChecked( id, t->isActive() );
		
	}
}

void TaskLMBMenu::dragEnterEvent( QDragEnterEvent* e )
{
	int id = idAt( e->pos() );
	
	if( id == -1 ) {
		dragSwitchTimer.stop();
		m_lastDragId = -1;
	} else if( id != m_lastDragId ) {
		m_lastDragId = id;
		dragSwitchTimer.start( 1000, TRUE );
	}
	
	QPopupMenu::dragEnterEvent( e );
}

void TaskLMBMenu::dragLeaveEvent( QDragLeaveEvent* e )
{
	dragSwitchTimer.stop();
	m_lastDragId = -1;
	
	QPopupMenu::dragLeaveEvent( e );
	
	hide();
}

void TaskLMBMenu::dragMoveEvent( QDragMoveEvent* e )
{
	int id = idAt( e->pos() );
	
	if( id == -1 ) {
		dragSwitchTimer.stop();
		m_lastDragId = -1;
	} else if( id != m_lastDragId ) {
		m_lastDragId = id;
		dragSwitchTimer.start( 1000, TRUE );
	}
	
	QPopupMenu::dragMoveEvent( e );
}

void TaskLMBMenu::dragSwitch()
{
	Task* t = m_tasks.at( indexOf( m_lastDragId ) );
	if( t ) {
		t->activate();
		for( int i = 0; i < count(); ++i )
			setItemChecked( idAt( i ), false );
		setItemChecked( m_lastDragId, true );
	}
}

