/*****************************************************************

Copyright (c) 2001 Matthias Elter <elter@kde.org>

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
{
	fillMenu( tasks );
}

void TaskLMBMenu::fillMenu( TaskList* tasks )
{
	setCheckable( true );
	
	for( QPtrListIterator<Task> it(*tasks); *it; ++it ) {
		Task* t = (*it);
		int id = insertItem( QIconSet( t->pixmap() ), 
                      t->visibleNameWithState().replace(QRegExp("&"), "&&"),
		                t, SLOT( activateRaiseOrIconify() ) );
		setItemChecked( id, t->isActive() );
	}
}
