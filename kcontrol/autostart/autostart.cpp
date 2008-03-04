/***************************************************************************
 *   Copyright (C) 2006-2007 by Stephen Leaf                               *
 *   smileaf@gmail.com                                                     *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA          *
 ***************************************************************************/

#include "autostart.h"

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
#include <KIO/CopyJob>

#include <QDir>
#include <QHeaderView>
#include <QItemDelegate>
#include <QTreeWidget>
#include <QGridLayout>
#include <QStringList>
#include <QStandardItemModel>

class Desktop : public QTreeWidgetItem {
public:
bool bisDesktop;
KUrl fileName;

Desktop( const QString &service, QTreeWidget *parent ): QTreeWidgetItem( parent, 1 ) {
	fileName = KUrl(service);
	bisDesktop = service.endsWith(".desktop");
}
bool isDesktop() { return bisDesktop; }
void setPath(const QString &path) {
	if (path == fileName.directory(KUrl::AppendTrailingSlash)) return;
	KIO::move(fileName, KUrl( path + '/' + fileName.fileName() ));
	fileName = KUrl(path + fileName.fileName());
}
~Desktop() {
}
};

K_PLUGIN_FACTORY(AutostartFactory, registerPlugin<Autostart>();)
K_EXPORT_PLUGIN(AutostartFactory( "kcmautostart" ))

Autostart::Autostart( QWidget* parent, const QVariantList& )
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

	KAboutData* about = new KAboutData("Autostart", 0, ki18n("KDE Autostart Manager"), "1.0",
		ki18n("KDE Autostart Manager Control Panel Module"),
		KAboutData::License_GPL,
		ki18n("(c) 2006-2007 Stephen Leaf"));
	about->addAuthor(ki18n("Stephen Leaf"), KLocalizedString(), "smileaf@gmail.com");
	setAboutData( about );
}


Autostart::~Autostart()
{}


void Autostart::load()
{
	// share/autostart may *only* contain .desktop files
	// shutdown and env may *only* contain scripts, links or binaries
	// autostart on the otherhand may contain all of the above.
	// share/autostart is special as it overrides entries found in $KDEDIR/share/autostart
	paths << KGlobalSettings::autostartPath()	// All new entries sholud go here
		  << componentData().dirs()->localkdedir() + "shutdown/"
		  << componentData().dirs()->localkdedir() + "env/"
		  << componentData().dirs()->localkdedir() + "share/autostart"	// For Importing purposes
		;

	// share/autostart shouldn't be an option as this should be reserved for global autostart entries
	pathName << i18n("Startup")
			 << i18n("Shutdown")
			 << i18n("Pre-KDE startup")
			 ;
	widget->cmbStartOn->addItems(pathName);
        widget->listCMD->clear();
	foreach (const QString& path, paths) {
		if (! KStandardDirs::exists(path))
			KStandardDirs::makeDir(path);

		QDir *autostartdir = new QDir( path );
		autostartdir->setFilter( QDir::Files );
		QFileInfoList list = autostartdir->entryInfoList();
		for (int i = 0; i < list.size(); ++i) {
			QFileInfo fi = list.at(i);
			QString filename = fi.fileName();
			Desktop *item = new Desktop( fi.absoluteFilePath(), widget->listCMD );
			if ( ! item->isDesktop() ) {
				if ( fi.isSymLink() ) {
					QString link = fi.readLink();
					item->setText( 0, filename );
					item->setText( 1, pathName.value(paths.indexOf((item->fileName.directory()+'/') )) );
					item->setText( 2, link );
				} else {
					item->setText( 0, filename );
					item->setText( 1, pathName.value(paths.indexOf((item->fileName.directory()+'/') )) );
					item->setText( 2, filename );
				}
			} else {
				KService  service(fi.absoluteFilePath());
				item->setText( 0, service.name() );
				item->setText( 1, pathName.value(paths.indexOf((item->fileName.directory()+'/') )) );
				item->setText( 2, service.exec() );
			}
		}
	}
}

void Autostart::addCMD() {
	AddDialog * addDialog = new AddDialog(this);
	int result = addDialog->exec();

	if (result == QDialog::Rejected) {
            delete addDialog;
            return;
	} else if (result == 3) {
		if (addDialog->symLink())
			KIO::link(addDialog->importUrl(), paths[0]);
		else
			KIO::copy(addDialog->importUrl(), paths[0]);

		Desktop * item = new Desktop( paths[0] + addDialog->importUrl().fileName(), widget->listCMD );
		item->setText( 0, addDialog->importUrl().fileName() );
		item->setText( 1, pathName.value(paths.indexOf((item->fileName.directory()+'/') )) );
		item->setText( 2, addDialog->importUrl().fileName() );
	} else if (result == 4) {
		KService::Ptr service;
		KOpenWithDialog owdlg( this );
		if (owdlg.exec() != QDialog::Accepted)
                {
                    delete addDialog;
                    return;
                }
		service = owdlg.service();

		Q_ASSERT(service);
		if (!service)
                {
                    delete addDialog;
                    return; // Don't crash if KOpenWith wasn't able to create service.
                }

		KUrl desktopTemplate;

		if ( service->desktopEntryName().isNull() ) {
			desktopTemplate = KUrl( kgs->autostartPath() + service->name() + ".desktop" );
			KConfig kc(desktopTemplate.path(), KConfig::SimpleConfig);
			KConfigGroup kcg = kc.group("Desktop Entry");
			kcg.writeEntry("Encoding","UTF-8");
			kcg.writeEntry("Exec",service->exec());
			kcg.writeEntry("Icon","system-run");
			kcg.writeEntry("Path","");
			kcg.writeEntry("Terminal",false);
			kcg.writeEntry("Type","Application");
			kc.sync();

			KPropertiesDialog dlg( desktopTemplate, this );
			if ( dlg.exec() != QDialog::Accepted )
                        {
                            delete addDialog;
                            return;
                        }
		} else {
			desktopTemplate = KUrl( KStandardDirs::locate("apps", service->entryPath()) );

			KPropertiesDialog dlg( desktopTemplate, KUrl(kgs->autostartPath()), service->name() + ".desktop", this );
			if ( dlg.exec() != QDialog::Accepted )
                        {
                            delete addDialog;
                            return;
                        }
		}
		Desktop * item = new Desktop( kgs->autostartPath() + service->name() + ".desktop", widget->listCMD );
		item->setText( 0, service->name() );
		item->setText( 1, pathName.value(paths.indexOf((item->fileName.directory()+'/') )) );
		item->setText( 2, service->exec() );
	}
        delete addDialog;
	emit changed(true);
}

void Autostart::removeCMD() {
	QList<QTreeWidgetItem *> list = widget->listCMD->selectedItems();
	if (list.isEmpty()) return;

	QStringList delList;
	foreach (QTreeWidgetItem *itm, list) {
		widget->listCMD->takeTopLevelItem( widget->listCMD->indexOfTopLevelItem( itm ) );
		delList.append( ((Desktop*)itm)->fileName.path() );
	}
	KIO::del( KUrl::List( delList ) );

	emit changed(true);
}
void Autostart::editCMD(QTreeWidgetItem* ent) {
	if (!ent) return;
	Desktop *entry = (Desktop*)ent;

	const KFileItem kfi = KFileItem( KFileItem::Unknown, KFileItem::Unknown, KUrl( entry->fileName ), true );
	if (! editCMD( kfi )) return;

	if (((Desktop*)entry)->isDesktop()) {
		KService service(entry->fileName.path());
		entry->setText( 0, service.name() );
		entry->setText( 1, pathName.value(paths.indexOf((entry->fileName.directory()+'/') )) );
		entry->setText( 2, service.exec() );
	}
}

bool Autostart::editCMD( const KFileItem &item) {
	KPropertiesDialog dlg( item, this );
	bool c = ( dlg.exec() == QDialog::Accepted );
	emit changed(c);
	return c;
}

void Autostart::editCMD() {
	if ( widget->listCMD->selectedItems().size() == 0 )
		return;
	editCMD( (Desktop*)widget->listCMD->selectedItems().first() );
}

void Autostart::setStartOn( int index ) {
	if ( widget->listCMD->selectedItems().size() == 0 )
		return;
	Desktop* entry = (Desktop*)widget->listCMD->selectedItems().first();
	entry->setPath(paths.value(index));
	entry->setText(1, pathName[index]);
}

void Autostart::selectionChanged() {
	bool hasItems = (widget->listCMD->selectedItems().size() != 0 );
	widget->cmbStartOn->setEnabled(hasItems);
	widget->btnRemove->setEnabled(hasItems);
	widget->btnProperties->setEnabled(hasItems);
	if (!hasItems)
		return;

	Desktop* entry = (Desktop*)widget->listCMD->selectedItems().first();
	widget->cmbStartOn->setCurrentIndex( paths.indexOf(entry->fileName.directory()+'/') );
}

void Autostart::defaults()
{
}

void Autostart::save()
{
}

#include "autostart.moc"
