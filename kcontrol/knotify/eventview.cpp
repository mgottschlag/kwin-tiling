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
#include <kiconloader.h>
#include <kinstance.h>




EventView::EventView(QWidget *parent, const char *name):
	QWidget(parent, name)
{
	QGridLayout *layout=new QGridLayout(this,2,3);
	
	static QStringList presentation;
	presentation << i18n("Sound")
	             << i18n("MessageBox")
	             << i18n("Log File")
	             << i18n("Standard Error");

	eventslist=new QListBox(this);
	eventslist->insertStringList(presentation);
	
	layout->addMultiCellWidget(eventslist, 0,2, 0,0);
	layout->addWidget(enabled=new QCheckBox(i18n("&Enabled"),this), 0,1);
	layout->addWidget(file=new KLineEdit(this), 1,1);
	layout->addWidget(todefault=new QPushButton(i18n("&Default"), this), 2,1);
	
	file->hide();
	connect(eventslist, SIGNAL(highlighted(int)), SLOT(itemSelected(int)));
	connect(enabled, SIGNAL(toggled(bool)), SLOT(itemToggled(bool)));
	connect(file, SIGNAL(textChanged(QString)), SLOT(changed(textChanged(QString))));
};

EventView::~EventView()
{
}

void EventView::defaults()
{
	emit changed();
}

void EventView::textChanged(const QString &str)
{
}
void EventView::itemSelected(int item)
{
}

void EventView::itemToggled(bool on)
{

}

void EventView::load(KConfig *config, const QString &section)
{

}

void EventView::setPixmap(int item, bool on)
{

}

void EventView::save()
{
}

void EventView::unload()
{

}
