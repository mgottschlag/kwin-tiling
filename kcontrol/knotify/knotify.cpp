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
#include <kstddirs.h>
#include <kiconloader.h>
#include <kmessagebox.h>

#include <qdir.h>
#include <qlayout.h>

#include "eventconfig.h"
#include "knotify.h"
#include "knotify.moc"

KNotifyWidget::KNotifyWidget(QWidget *parent, const char *name):
	KCModule(parent, name)
{
	QVBoxLayout *layout=new QVBoxLayout(this,0,3);
	
	apps=new QListView(this);
	apps->addColumn(i18n("Application Name"));
	apps->addColumn(i18n("Description"));
	apps->setSelectionMode(QListView::Single);
	layout->addWidget(apps, 1);
	
	events=new QListView(this);
	events->setSelectionMode(QListView::Single);
	events->addColumn(i18n("Event Name"));
	events->addColumn(i18n("Description"));
	layout->addWidget(events, 1);
	
	eventview=new EventView(this);
	eventview->setEnabled(false);
	layout->addWidget(eventview, 1);
	loadAll();
};

KNotifyWidget::~KNotifyWidget()
{

}

void KNotifyWidget::defaults()
{
	if (KMessageBox::warningContinueCancel(this,
		i18n("This will cause the notifications for *All Applications* "
		     "to be reset to their defaults!"), i18n("Are you sure?!"), i18n("Continue"))
		!= KMessageBox::Continue)
		return;
	delete applications;
	loadAll();
}

void KNotifyWidget::changed()
{
	emit KCModule::changed(true);
}

void KNotifyWidget::loadAll()
{
	applications = new Programs(eventview, apps, events);
	applications->show();
}

void KNotifyWidget::save()
{
	if (applications)
		applications->save();

}

QString KNotifyWidget::quickHelp()
{
	return i18n("Configure system notifications.  Set what happens when a certain event is triggered.");
}

