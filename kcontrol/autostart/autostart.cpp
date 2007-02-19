/***************************************************************************
 *   Copyright (C) 2006-2007 by Stephen Leaf                               *
 *   smileaf@smileaf.org                                                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include <KGenericFactory>
#include <KLocale>
#include <KGlobal>
#include <KSimpleConfig>
#include <KGlobalSettings>
#include <KStandardDirs>
#include <KUrlRequester>
#include <KOpenWithDialog>
#include <KPropertiesDialog>
#include <KIO/NetAccess>
#include <KIO/DeleteJob>

#include <QDir>
#include <QHeaderView>
#include <QTreeWidget>
#include <QGridLayout>

#include "autostart.h"

class desktop : public QTreeWidgetItem {
public:
KService * service;
bool bisDesktop;
KUrl fileName;
int iStartOn;
enum { AutoStart, Shutdown, ENV };

desktop( QString service, int startOn, QTreeWidget *parent ): QTreeWidgetItem( parent ) {
	bisDesktop = false;
	iStartOn = startOn;
	fileName = KUrl(service);
	if (service.endsWith(".desktop")) {
		this->service = new KService(service);
		bisDesktop = true;
	}
}
bool isDesktop() { return bisDesktop; }
int startOn() { return iStartOn; }
QString fStartOn() {
	switch (iStartOn) {
		case AutoStart: return i18n("Startup"); break;
		case Shutdown: return i18n("Shutdown"); break;
		case ENV: return i18n("Pre-Desktop"); break;
		default: return ""; break;
	}
}
void setStartOn(int start) {
	iStartOn = start;
	setText(2, fStartOn() );
	QString path;
	KStandardDirs *ksd = new KStandardDirs();
	switch (iStartOn) {
		case AutoStart: path = KGlobalSettings::autostartPath()+"/"; break;
		case Shutdown: path = ksd->localkdedir()+"shutdown/"; break;
		case ENV: path = ksd->localkdedir()+"env/"; break;
	}
	KIO::NetAccess::file_move(fileName, KUrl( path + fileName.fileName() ));
	fileName = path + fileName.fileName();
}
void updateService() {
	if (bisDesktop) service = new KService( fileName.path() );
}
~desktop() {
	delete service;
}
};

typedef KGenericFactory<autostart, QWidget> autostartFactory;
K_EXPORT_COMPONENT_FACTORY( autostart, autostartFactory("kcmautostart"))

autostart::autostart( QWidget* parent, const QStringList& )
    : KCModule( autostartFactory::componentData(), parent ), myAboutData(0)
{
	widget = new Ui_AutostartConfig();
	widget->setupUi(this);

	connect( widget->btnAdd, SIGNAL(clicked()), SLOT(addCMD()) );
	connect( widget->btnRemove, SIGNAL(clicked()), SLOT(removeCMD()) );
	connect( widget->listCMD, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)), SLOT(editCMD(QTreeWidgetItem*)) );
	connect( widget->btnProperties, SIGNAL(clicked()), SLOT(editCMD()) );
	connect( widget->cmbStartOn, SIGNAL(activated(int)), SLOT(setStartOn(int)) );
	connect( widget->listCMD, SIGNAL(itemSelectionChanged()), SLOT(selectionChanged()) );

	widget->listCMD->setFocus();

    load();

	KAboutData* about = new KAboutData("autostart", I18N_NOOP("KDE Autostart Manager"), "1.0",
		I18N_NOOP("KDE Autostart Manager Control Panel Module"),
		KAboutData::License_GPL,
		I18N_NOOP("(c) 2006-2007 Stephen Leaf"), 0, 0);
	about->addAuthor("Stephen Leaf", 0, "smileaf@smileaf.org");
	setAboutData( about );
};


autostart::~autostart()
{}


void autostart::load()
{
	KStandardDirs *ksd = componentData().dirs();

	QStringList paths;
	paths << KGlobalSettings::autostartPath()
		  << ksd->localkdedir() + "/shutdown"
		  << ksd->localkdedir() + "/env";
	int x=0;  // Required for specifying the location of the desktop within our object.
			  // Paths should be in the same order as the enum found in the desktop object.
	foreach (const QString& path, paths) {

		if (! KStandardDirs::exists(path)) KStandardDirs::makeDir(path);

		QDir *autostartdir = new QDir( path );
		autostartdir->setFilter( QDir::Files );
		QFileInfoList list = autostartdir->entryInfoList();
		for (int i = 0; i < list.size(); ++i) {
			QFileInfo fi = list.at(i);
			QString filename = fi.fileName();
			desktop * item = new desktop( fi.absoluteFilePath(), x, widget->listCMD );
			if ( ! item->isDesktop() ) {
				if ( fi.isSymLink() ) {
					QString link = fi.readLink();
					item->setText( 0, filename );
					item->setText( 1, link );
					item->setText( 2, item->fStartOn() );
				} else {
					item->setText( 0, filename );
					item->setText( 1, filename );
					item->setText( 2, item->fStartOn() );
				}
			} else {
				item->setText( 0, item->service->name() );
				item->setText( 1, item->service->exec() );
				item->setText( 2, item->fStartOn() );
			}
		}
		x++;
	}
}

void autostart::addCMD() {
	KService::Ptr service;
	KOpenWithDialog owdlg( this );
	if (owdlg.exec() != QDialog::Accepted)
		return;
	service = owdlg.service();

	Q_ASSERT(service);
	if (!service)
		return; // Don't crash if KOpenWith wasn't able to create service.


	KUrl desktopTemplate;

	if ( service->desktopEntryName().isNull() ) {
		desktopTemplate = KUrl( kgs->autostartPath() + service->name() + ".desktop" );
		KSimpleConfig ksc(desktopTemplate.path());
		ksc.setGroup("Desktop Entry");
		ksc.writeEntry("Encoding","UTF-8");
		ksc.writeEntry("Exec",service->exec());
		ksc.writeEntry("Icon","exec");
		ksc.writeEntry("Path","");
		ksc.writeEntry("Terminal",false);
		ksc.writeEntry("Type","Application");
		ksc.sync();

		// FIXME: Make it so you can't give focus to the parent before this goes away.
		// If the parent closes before this does, a crash is generated.
		KPropertiesDialog dlg( desktopTemplate, this );
		if ( dlg.exec() != QDialog::Accepted )
			return;
	} else {
		desktopTemplate = KUrl( KStandardDirs::locate("apps", service->desktopEntryPath()) );

		// FIXME: Make it so you can't give focus to the parent before this goes away.
		// If the parent closes before this does, a crash is generated.
		KPropertiesDialog dlg( desktopTemplate, KUrl(kgs->autostartPath()), service->name() + ".desktop", this );
		if ( dlg.exec() != QDialog::Accepted )
			return;
	}

	desktop * item = new desktop( kgs->autostartPath() + service->name() + ".desktop", desktop::AutoStart, widget->listCMD );
	item->setText( 0, item->service->name() );
	item->setText( 1, item->service->exec() );
	item->setText( 2, item->fStartOn() );
	emit changed(true);
}

void autostart::removeCMD() {
	QList<QTreeWidgetItem *> list = widget->listCMD->selectedItems();
	if (list.isEmpty()) return;
	
	QStringList delList;
	foreach (QTreeWidgetItem *itm, list) {
		widget->listCMD->takeTopLevelItem( widget->listCMD->indexOfTopLevelItem( itm ) );
		delList.append( ((desktop *)itm)->fileName.path() );
	}
	KIO::del( KUrl::List( delList ) );

	emit changed(true);
}

void autostart::editCMD(QTreeWidgetItem* entry) {
	if (!entry) return;

	((desktop*)entry)->updateService();
	KFileItem kfi = KFileItem( KFileItem::Unknown, KFileItem::Unknown, KUrl( ((desktop*)entry)->fileName ), true );
	if (! editCMD( kfi )) return;

	// Remove and recreate
	if (((desktop*)entry)->isDesktop()) {
		widget->listCMD->takeTopLevelItem( widget->listCMD->indexOfTopLevelItem(widget->listCMD->selectedItems().first()) );
		desktop * item = new desktop( ((desktop*)entry)->fileName.path(), ((desktop*)entry)->startOn(), widget->listCMD );
		item->setText( 0, ((desktop*)entry)->service->name() );
		item->setText( 1, ((desktop*)entry)->service->exec() );
		item->setText( 2, ((desktop*)entry)->fStartOn() );
	}
}

bool autostart::editCMD( KFileItem item) {
	KPropertiesDialog dlg( &item, this );
	if ( dlg.exec() != QDialog::Accepted )
		return false;

	emit changed(true);
	return true;
}

void autostart::editCMD() {
	if ( widget->listCMD->selectedItems().size() == 0 ) return;
	editCMD( widget->listCMD->selectedItems().first() );
}

void autostart::setStartOn( int index ) {
	if ( widget->listCMD->selectedItems().size() == 0 ) return;
	((desktop*)widget->listCMD->selectedItems().first())->setStartOn(index);
}

void autostart::selectionChanged() {
	if ( widget->listCMD->selectedItems().size() == 0 ) {
		widget->cmbStartOn->setEnabled(false);
		widget->btnRemove->setEnabled(false);
		widget->btnProperties->setEnabled(false);
		return;
	} else {
		widget->cmbStartOn->setEnabled( true );
		widget->btnRemove->setEnabled(true);
		widget->btnProperties->setEnabled(true);
	}
	QTreeWidgetItem* entry = widget->listCMD->selectedItems().first();
	widget->cmbStartOn->setCurrentIndex( ((desktop*)entry)->startOn() );
}

void autostart::defaults()
{
}

void autostart::save()
{
}

#include "autostart.moc"
