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
#include <KConfig>
#include <KConfigGroup>
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
#include <QStringList>

#include "autostart.h"

class Desktop : public QTreeWidgetItem {
public:
bool bisDesktop;
KUrl fileName;

Desktop( QString service, QTreeWidget *parent ): QTreeWidgetItem( parent ) {
	fileName = KUrl(service);
	bisDesktop = service.endsWith(".desktop");
}
bool isDesktop() { return bisDesktop; }
void setPath(QString path) {
	KIO::NetAccess::file_move(fileName, KUrl( path + '/' + fileName.fileName() ));
	fileName = KUrl(path + fileName.fileName());
}
~Desktop() {
}
};

typedef KGenericFactory<Autostart, QWidget> AutostartFactory;
K_EXPORT_COMPONENT_FACTORY( Autostart, AutostartFactory("kcmautostart"))

Autostart::Autostart( QWidget* parent, const QStringList& )
    : KCModule( AutostartFactory::componentData(), parent ), myAboutData(0)
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

	KAboutData* about = new KAboutData("Autostart", I18N_NOOP("KDE Autostart Manager"), "1.0",
		I18N_NOOP("KDE Autostart Manager Control Panel Module"),
		KAboutData::License_GPL,
		I18N_NOOP("(c) 2006-2007 Stephen Leaf"), 0, 0);
	about->addAuthor("Stephen Leaf", 0, "smileaf@smileaf.org");
	setAboutData( about );
};


Autostart::~Autostart()
{}


void Autostart::load()
{
	// share/autostart may *only* contain .desktop files
	// shutdown and env may *only* contain scripts, links or binaries
	// autostart on the otherhand may contain all of the above.
	// share/autostart is special as it overrides entries found in $KDEDIR/share/autostart
	paths << KGlobalSettings::autostartPath()
		  << componentData().dirs()->localkdedir() + "share/autostart"
//		  << componentData().dirs()->localkdedir() + "shutdown/"
//		  << componentData().dirs()->localkdedir() + "env/"
		;

	// share/autostart shouldn't be an option as this should be reserved for global autostart entries
	pathName << i18n("Autostart")
// 			 << i18n("Shutdown")
// 			 << i18n("Pre-Desktop")
			 ;
	widget->cmbStartOn->addItems(pathName);

	foreach (const QString& path, paths) {
		if (! KStandardDirs::exists(path))
			KStandardDirs::makeDir(path);

		QDir *autostartdir = new QDir( path );
		autostartdir->setFilter( QDir::Files );
		QFileInfoList list = autostartdir->entryInfoList();
		for (int i = 0; i < list.size(); ++i) {
			QFileInfo fi = list.at(i);
			QString filename = fi.fileName();
			Desktop * item = new Desktop( fi.absoluteFilePath(), widget->listCMD );
			if ( ! item->isDesktop() ) {
				if ( fi.isSymLink() ) {
					QString link = fi.readLink();
					item->setText( 0, filename );
					item->setText( 1, link );
					item->setText( 2, pathName.value(paths.indexOf((item->fileName.directory()+'/') )) );
				} else {
					item->setText( 0, filename );
					item->setText( 1, filename );
					item->setText( 2, pathName.value(paths.indexOf((item->fileName.directory()+'/') )) );
				}
			} else {
				KService * service = new KService(fi.absoluteFilePath());
				item->setText( 0, service->name() );
				item->setText( 1, service->exec() );
				item->setText( 2, pathName.value(paths.indexOf((item->fileName.directory()+'/') )) );
			}
		}
	}
}

void Autostart::addCMD() {
	AddDialog * addDialog = new AddDialog(this);
	int result = addDialog->exec();
	if (result == QDialog::Rejected)
		return;
	else if (result == 4) {
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
			KConfig kc(desktopTemplate.path(), KConfig::OnlyLocal);
			KConfigGroup kcg = kc.group("Desktop Entry");
			kcg.writeEntry("Encoding","UTF-8");
			kcg.writeEntry("Exec",service->exec());
			kcg.writeEntry("Icon","exec");
			kcg.writeEntry("Path","");
			kcg.writeEntry("Terminal",false);
			kcg.writeEntry("Type","Application");
			kc.sync();

			KPropertiesDialog dlg( desktopTemplate, this );
			if ( dlg.exec() != QDialog::Accepted )
				return;
		} else {
			desktopTemplate = KUrl( KStandardDirs::locate("apps", service->desktopEntryPath()) );

			KPropertiesDialog dlg( desktopTemplate, KUrl(kgs->autostartPath()), service->name() + ".desktop", this );
			if ( dlg.exec() != QDialog::Accepted )
				return;
		}

		Desktop * item = new Desktop( kgs->autostartPath() + service->name() + ".desktop", widget->listCMD );
		item->setText( 0, service->name() );
		item->setText( 1, service->exec() );
		item->setText( 2, pathName.value(paths.indexOf((item->fileName.directory()+'/') )) );
	}
	emit changed(true);
}

void Autostart::removeCMD() {
	QList<QTreeWidgetItem *> list = widget->listCMD->selectedItems();
	if (list.isEmpty()) return;

	QStringList delList;
	foreach (QTreeWidgetItem *itm, list) {
		widget->listCMD->takeTopLevelItem( widget->listCMD->indexOfTopLevelItem( itm ) );
		delList.append( ((Desktop *)itm)->fileName.path() );
	}
	KIO::del( KUrl::List( delList ) );

	emit changed(true);
}

void Autostart::editCMD(QTreeWidgetItem* entry) {
	if (!entry) return;

	KFileItem kfi = KFileItem( KFileItem::Unknown, KFileItem::Unknown, KUrl( ((Desktop*)entry)->fileName ), true );
	if (! editCMD( kfi )) return;

	if (((Desktop*)entry)->isDesktop()) {
		KService * service = new KService(((Desktop*)entry)->fileName.path());
		entry->setText( 0, service->name() );
		entry->setText( 1, service->exec() );
		entry->setText( 2, pathName.value(paths.indexOf((((Desktop*)entry)->fileName.directory()+'/') )) );
	}
}

bool Autostart::editCMD( KFileItem item) {
	KPropertiesDialog dlg( &item, this );
	bool c = ( dlg.exec() == QDialog::Accepted );
	emit changed(c);
	return c;
}

void Autostart::editCMD() {
	if ( widget->listCMD->selectedItems().size() == 0 )
		return;
	editCMD( widget->listCMD->selectedItems().first() );
}

void Autostart::setStartOn( int index ) {
	if ( widget->listCMD->selectedItems().size() == 0 )
		return;
	Desktop* entry = (Desktop*)widget->listCMD->selectedItems().first();
	entry->setPath(paths.value(index));
	entry->setText(2, entry->fileName.directory() );
}

void Autostart::selectionChanged() {
	bool hasItems = (widget->listCMD->selectedItems().size() != 0 );
	widget->cmbStartOn->setEnabled(hasItems);
	widget->btnRemove->setEnabled(hasItems);
	widget->btnProperties->setEnabled(hasItems);
	if (!hasItems)
		return;

	QTreeWidgetItem* entry = widget->listCMD->selectedItems().first();
	widget->cmbStartOn->setCurrentIndex( paths.indexOf(((Desktop*)entry)->fileName.directory()+'/') );
}

void Autostart::defaults()
{
}

void Autostart::save()
{
}

#include "autostart.moc"
