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

static QString presentation[5] = {
	i18n("Sound"),
	i18n("MessageBox"),
	i18n("Log Window"),
	i18n("Log File"),
	i18n("Standard Error")
};


EventView::EventView(QWidget *parent, const char *name):
	QWidget(parent, name), conf(0)
{
	QGridLayout *layout=new QGridLayout(this,2,3);
	
	eventslist=new QListBox(this);
	{	eventslist->insertItem(KGlobal::instance()->iconLoader()->loadIcon("toolbars/flag", KIconLoader::Small), presentation[0]);
		eventslist->insertItem(KGlobal::instance()->iconLoader()->loadIcon("toolbars/flag", KIconLoader::Small),presentation[1]);
		eventslist->insertItem(KGlobal::instance()->iconLoader()->loadIcon("toolbars/flag", KIconLoader::Small),presentation[2]);
		eventslist->insertItem(KGlobal::instance()->iconLoader()->loadIcon("toolbars/flag", KIconLoader::Small),presentation[3]);
		eventslist->insertItem(KGlobal::instance()->iconLoader()->loadIcon("toolbars/flag", KIconLoader::Small),presentation[4]);
	}
	
	layout->addMultiCellWidget(eventslist, 0,2, 0,0);
	layout->addWidget(enabled=new QCheckBox(i18n("&Enabled"),this), 0,1);
	layout->addWidget(file=new KLineEdit(this), 1,1);
	layout->addWidget(todefault=new QPushButton(i18n("&Default"), this), 2,1);
};

EventView::~EventView()
{
}

void EventView::defaults()
{

}

void EventView::load(KConfig *config, const QString &section)
{
	unload();
	conf=config;
	this->section=section;
	setEnabled(true);
	
}

void EventView::save()
{
	unload();
}

void EventView::unload()
{
	
	delete conf;
	enabled->setChecked(false);
	file->setText("");
	setEnabled(false);
}
