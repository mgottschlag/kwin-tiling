/*

    $Id$


    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.

*/

#include "knotify.h"
#include "knotify.moc"

#include <qlabel.h>
#include <qlayout.h>
#include <X11/Xlib.h>
#include <config.h>

KNotifyWidget::KNotifyWidget(QWidget *parent, const char *name):
	KCModule(parent, name)
{
	QVBoxLayout *layout=new QVBoxLayout(this,0,5);
	layout->setAutoAdd(true);
	
	apps=new QListView(this);
	events=new QListView(this);
	eventview=new EventView(this);
};

KNotifyWidget::~KNotifyWidget()
{
}

void KNotifyWidget::defaults()
{
}

void KNotifyWidget::changed()
{
  emit KCModule::changed(true);
}
