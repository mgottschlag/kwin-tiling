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
#include "eventconfig.h"



EventView::EventView(QWidget *parent, const char *name):
	QWidget(parent, name), event(0)
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
	(void)str;
}
void EventView::itemSelected(int item)
{
	if (event->present & enumNum(item))
		enabled->setChecked(true);
	
	if (enumNum(item) & KNotifyClient::Sound)
		file->show(), file->setText(event->soundfile);
	
	if (enumNum(item) & KNotifyClient::Logfile)
		file->show(), file->setText(event->logfile);
		
}

void EventView::itemToggled(bool on)
{
	(void)on;
}

void EventView::load(EventConfig *_event)
{
	unload();
	event=_event;
	setEnabled(true);
	
	setPixmaps();
	eventslist->setSelected(0, true);
	kapp->processEvents();
	eventslist->setContentsPos(0,0); // go to the top
}

void EventView::setPixmaps()
{ // Handle all of 'dem at once
	int i=0;
	for (int c=1; c <=8; c*=2)
	{
		if ( event && (event->present & c))
			eventslist->changeItem(SmallIcon("flag"), eventslist->text(i), i);
		else
			eventslist->changeItem(eventslist->text(i), i);
		i++;
	}
}

void EventView::unload(bool save)
{
	(void)save;
}

int EventView::listNum(int enumNum)
{
	switch (enumNum)
	{
	case (1): return 0;
	case (2): return 1;
	case (4): return 2;
	case (8): return 3;
	default: return 1;
	}
}

int EventView::enumNum(int listNum)
{
	switch (listNum)
	{
	case (0): return 1;
	case (1): return 2;
	case (2): return 4;
	case (3): return 8;
	default: return 0;
	}
}
