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

#include <kapp.h>
#include <kmessagebox.h>
#include <klocale.h>
#include "knotify.h"
#include "knotify.moc"
#include <kstddirs.h>
#include <kiconloader.h>
#include <qdir.h>
#include <stdio.h>

#include <qlayout.h>

KNotifyWidget::KNotifyWidget(QWidget *parent, const char *name):
	KCModule(parent, name)
{
	QVBoxLayout *layout=new QVBoxLayout(this,0,3);
	//layout->setAutoAdd(true);
	
	apps=new QListView(this);
	//apps->setMaximumHeight(100);
	apps->addColumn(i18n("Application Name"));
	apps->addColumn(i18n("Description"));
	apps->setSelectionMode(QListView::Single);
	layout->addWidget(apps, 1);
	//layout->setStretchFactor(events, -1);
	
	events=new QListView(this);
	events->setSelectionMode(QListView::Single);
	events->addColumn(i18n("Event Name"));
	events->addColumn(i18n("Description"));
	layout->addWidget(events, 1);
	//layout->setStretchFactor(events, 1);
	
	eventview=new EventView(this);
	eventview->setEnabled(false);
	layout->addWidget(eventview, 1);
	//layout->setStretchFactor(events, 1);
	
	connect(apps, SIGNAL(selectionChanged(QListViewItem*)),
	        SLOT(appSelected(QListViewmItem*)));
	connect(apps, SIGNAL(selectionChanged(QListViewItem*)), 
		SLOT(appSelected(QListViewItem*)));
	connect(events, SIGNAL(selectionChanged(QListViewItem*)),
		SLOT(eventSelected(QListViewItem*)));
	
	loadAll();
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

void KNotifyWidget::loadAll()
{
	QStringList dirs(locate("config", "eventsrc")); // load system-wide eventsrc
	dirs+=KGlobal::dirs()->findAllResources("data", "*/eventsrc");
	
	for (QStringList::Iterator it=dirs.begin(); it!=dirs.end(); ++it)
	{
		if (!QFileInfo(*it).isReadable()) continue;
		KConfig conf(*it);
		conf.setGroup("!Global!");
		QString appname(conf.readEntry("appname", "Unknown Title"));
		QString desc(conf.readEntry("description"));
		(new ListViewItem(apps, *it, appname, desc))->setPixmap
			(0, SmallIcon("library"));
		kapp->processEvents();
	}
	if (!apps->firstChild()) apps->setEnabled(false);
	apps->setSelected(apps->firstChild(), true);
	appSelected(apps->firstChild());
}

void KNotifyWidget::appSelected(QListViewItem *_i)
{
	if (!_i)
	{
		events->setEnabled(false);
		return;
	}
	events->clear();
	// Set the rest of the dialog to show the proper info
	printf("appSelected : %s\n\n", (const char*)((ListViewItem*)_i)->file) ;
	KConfig conf(((ListViewItem*)_i)->file,false,false);
	QStringList eventlist=conf.groupList();
	eventlist.remove(QString("!Global!"));
	eventlist.remove(QString("<default>"));
	
	for (QStringList::Iterator it=eventlist.begin(); it!=eventlist.end(); ++it)
	{
		conf.setGroup(*it);
		QString friendly(conf.readEntry("friendly", *it));
		QString desc(conf.readEntry("description"));
		
		(new EventListViewItem(events, *it, (const char*)((ListViewItem*)_i)->file, friendly,
			desc))->setPixmap(0, SmallIcon("knotify"));
		kapp->processEvents();
	}

}

void KNotifyWidget::eventSelected(QListViewItem *_i)
{
	if (!_i)
	{
		eventview->setEnabled(false);
		return;
	}
	
	QString file=((EventListViewItem*)_i)->file;
	QString event=((EventListViewItem*)_i)->event;
	
	KConfig *conf = new KConfig(file,false,false);
	KMessageBox::error(this, file, "");
	eventview->load(conf,event);
}



ListViewItem::ListViewItem(QListView *parent, const QString &configfile,
	const QString &r1, const QString &r2)
	: QListViewItem(parent, r1,r2), file(configfile)
{}

EventListViewItem::EventListViewItem(QListView *parent, const QString &eventname,
	const QString &configfile, const QString &r1, const QString &r2)
	: QListViewItem(parent, r1,r2), file(configfile), event(eventname)
{}


