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
	(new QVBoxLayout(this,0,3))->setAutoAdd(true);
	
	apps=new QListView(this);
	apps->addColumn(i18n("Application Name"));
	apps->addColumn(i18n("Description"));
	
	events=new QListView(this);
	events->addColumn(i18n("Event Name"));
	events->addColumn(i18n("Description"));
	eventview=new EventView(this);
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
	printf("KNotifyWidget::loadAll()\n");
	QStringList dirs = KGlobal::dirs()->findAllResources("data", "*/eventsrc");
	for (QStringList::Iterator it=dirs.begin(); it!=dirs.end(); ++it)
	{
		if (!QFileInfo(*it).isReadable()) continue;
		KConfig conf(*it);
		conf.setGroup("!Global!");
		QString appname(conf.readEntry("appname", "Unknown Title"));
		QString desc(conf.readEntry("description"));
		(new QListViewItem(apps, *it, appname, desc))->setPixmap(0, KGlobal::instance()->iconLoader()->loadIcon("apps/library", KIconLoader::Small));
		kapp->processEvents();
	}
	appSelected(apps->firstChild());
}

void KNotifyWidget::appSelected(QListViewItem *)
{
	// Set the rest of the dialog to show the proper info


}

ListViewItem::ListViewItem(QListView *parent, const QString &configfile, const QString &r1, const QString &r2)
	: QListViewItem(parent, r1,r2), file(configfile)
{
}






