/*

    $Id$

    Copyright (C) 2000 Charles Samuels <charles@altair.dhs.org>

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

#include "eventview.h"
#include "eventview.moc"

#include <qlayout.h>
#include <klocale.h>


EventView::EventView(QWidget *parent, const char *name):
	QWidget(parent, name)
{
	QGridLayout *layout=new QGridLayout(this,2,2);
	eventslist=new QListBox(this);
	layout->addMultiCellWidget(eventslist, 0,1, 0,0);
	layout->addWidget(enabled=new QCheckBox(i18n("&Enabled"),this), 0,1);
	layout->addWidget(file=new KLineEdit(this), 1,1);
};

EventView::~EventView()
{
}

void EventView::defaults()
{

}

void EventView::load(KConfig *config, const QString &section)
{

}

void EventView::save()
{

}

void EventView::unload()
{

}

