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
#include <qsplitter.h>

#include "eventconfig.h"
#include "knotify.h"
#include "knotify.moc"

KNotifyWidget::KNotifyWidget(QWidget *parent, const char *name):
	KCModule(parent, name)
{
	(new QHBoxLayout(this))->setAutoAdd(true);
	QSplitter *split=new QSplitter(Vertical, this);
	
	apps=new QListView(split);
	apps->addColumn(i18n("Application Name"));
	apps->addColumn(i18n("Description"));
	apps->setSelectionMode(QListView::Single);
	
	events=new QListView(split);
	events->setSelectionMode(QListView::Single);
	events->addColumn(i18n("Event Name"));
	events->addColumn(i18n("Description"));
	
	eventview=new EventView(split);
	eventview->setEnabled(false);
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
	return i18n("<h1>System Notifications</h1>"
		    "KDE allows for a great deal of control over how you "
		    "will be notified when certain events occur.  There are "
		    "several choices as to how you are notified:"
		    "<ul><li>As the application was originally designed."
		    "<li>With a beep or other noise."
		    "<li>Via a popup dialog box with additional information."
		    "<li>By recording the the event in a logfile without "
		    "any additional visual or auditory alert."
		    "</ul>");
}

const KAboutData *KNotifyWidget::aboutData() const
{
	KAboutData ab(
		"kcmnotify", I18N_NOOP("KNotify"), "2.0pre",
		I18N_NOOP("System Notification Control Panel Module"),
		KAboutData::License_GPL, I18N_NOOP("(c) 2000 Charles Samuels"),
		0, 0, "charles@kde.org");
	return &ab;

}
