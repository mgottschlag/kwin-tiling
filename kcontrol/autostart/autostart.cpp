/***************************************************************************
 *   Copyright (C) 2006-2007 by Stephen Leaf                               *
 *   smileaf@gmail.com                                                     *
 *   Copyright (C) 2008 by Montel Laurent <montel@kde.org>                 *
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
#include <KDesktopFile>
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
    Desktop( const QString &service, QTreeWidget *parent )
        : QTreeWidgetItem( parent, 1 )
    {
        m_fileName = KUrl(service);
        m_isDesktop = service.endsWith(".desktop");
        if ( m_isDesktop )
            setCheckState ( 3,Qt::Checked );
    }
    bool isDesktop() const
    {
        return m_isDesktop;
    }

    KUrl fileName() const
    {
        return m_fileName;
    }

    void setPath(const QString &path) {
	if (path == m_fileName.directory(KUrl::AppendTrailingSlash))
            return;
	KIO::move(m_fileName, KUrl( path + '/' + m_fileName.fileName() ));
	m_fileName = KUrl(path + m_fileName.fileName());
    }
    ~Desktop()
    {
    }
private:
    bool m_isDesktop;
    KUrl m_fileName;
};

K_PLUGIN_FACTORY(AutostartFactory, registerPlugin<Autostart>();)
    K_EXPORT_PLUGIN(AutostartFactory( "kcmautostart" ))

    Autostart::Autostart( QWidget* parent, const QVariantList& )
        : KCModule( AutostartFactory::componentData(), parent )
{
    widget = new Ui_AutostartConfig();
    widget->setupUi(this);
    QStringList lstHeader;
    lstHeader<<i18n( "Name" )<<i18n( "Run On" )<< i18n( "Command" )<< i18n( "Status" );
    widget->listCMD->setHeaderLabels(lstHeader);
    setButtons(Apply);
    connect( widget->btnAddScript, SIGNAL(clicked()), SLOT(slotAddCMD()) );
    connect( widget->btnAddProgram, SIGNAL(clicked()), SLOT(slotAddProgram()) );
    connect( widget->btnRemove, SIGNAL(clicked()), SLOT(slotRemoveCMD()) );
    connect( widget->listCMD, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)), SLOT(slotEditCMD(QTreeWidgetItem*)) );
    connect( widget->listCMD, SIGNAL(itemClicked(QTreeWidgetItem *, int) ),this,SLOT( slotItemClicked( QTreeWidgetItem *, int) ) );
    connect( widget->btnProperties, SIGNAL(clicked()), SLOT(slotEditCMD()) );
    connect( widget->cmbStartOn, SIGNAL(activated(int)), SLOT(slotSetStartOn(int)) );
    connect( widget->listCMD, SIGNAL(itemSelectionChanged()), SLOT(slotSelectionChanged()) );

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
{
   delete widget;
}


void Autostart::slotItemClicked( QTreeWidgetItem *item, int col)
{
    if ( item && col == COL_STATUS )
    {
        Desktop *entry = static_cast<Desktop*>( item );
        if ( entry->isDesktop() )
        {
            bool disable = ( item->checkState( col ) == Qt::Unchecked );
            KDesktopFile kc(entry->fileName().path());
            KConfigGroup grp = kc.desktopGroup();
            grp.writeEntry("Hidden", !disable);
            kc.sync();
            if ( disable )
                item->setText( COL_STATUS, i18n( "Disabled" ) );
            else
                item->setText( COL_STATUS, i18n( "Enabled" ) );
        }
    }
}

void Autostart::addItem( QTreeWidgetItem*item, const QString& name, const QString& run, const QString& command, bool status )
{
    Q_ASSERT( item );
    item->setText( COL_NAME, name );
    item->setText( COL_RUN, run );
    item->setText( COL_COMMAND, command );
    Desktop *entry = static_cast<Desktop*>( item );
    if ( entry->isDesktop() )
    {
        item->setCheckState( COL_STATUS, status ? Qt::Checked : Qt::Unchecked );
        item->setText( COL_STATUS, status ? i18n( "Enabled" ) : i18n( "Disabled" ) );
    }
}

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

        QDir autostartdir( path );
        autostartdir.setFilter( QDir::Files );
        const QFileInfoList list = autostartdir.entryInfoList();
        for (int i = 0; i < list.size(); ++i) {
            QFileInfo fi = list.at(i);
            QString filename = fi.fileName();
            Desktop *item = new Desktop( fi.absoluteFilePath(), widget->listCMD );
            if ( ! item->isDesktop() ) {
                if ( fi.isSymLink() ) {
                    QString link = fi.readLink();
                    addItem(item, filename, pathName.value(paths.indexOf((item->fileName().directory()+'/') )), link );
                } else {
                    addItem( item, filename, pathName.value(paths.indexOf((item->fileName().directory()+'/') )), filename );
                }
            } else {
                KDesktopFile config(fi.absoluteFilePath());
                const KConfigGroup grp = config.desktopGroup();
                bool status = grp.readEntry("Hidden", false);
                addItem(item, config.readName(), pathName.value(paths.indexOf((item->fileName().directory()+'/') )),  grp.readEntry("Exec"),status );
            }
        }
    }
    //Update button
    slotSelectionChanged();
}

void Autostart::slotAddProgram()
{
    KService::Ptr service;
    KOpenWithDialog owdlg( this );
    if (owdlg.exec() != QDialog::Accepted)
        return;
    service = owdlg.service();

    Q_ASSERT(service);
    if (!service)
    {
        return; // Don't crash if KOpenWith wasn't able to create service.
    }

    KUrl desktopTemplate;
    if ( service->desktopEntryName().isEmpty() ) {
        desktopTemplate = KUrl( KGlobalSettings::autostartPath() + service->name() + ".desktop" );
        KConfig kc(desktopTemplate.path(), KConfig::SimpleConfig);
        KConfigGroup kcg = kc.group("Desktop Entry");
        kcg.writeEntry("Exec",service->exec());
        kcg.writeEntry("Icon","system-run");
        kcg.writeEntry("Path","");
        kcg.writeEntry("Terminal",false);
        kcg.writeEntry("Type","Application");
        kc.sync();

        KPropertiesDialog dlg( desktopTemplate, this );
        if ( dlg.exec() != QDialog::Accepted )
        {
            return;
        }
    }
    else
    {
        desktopTemplate = KUrl( KStandardDirs::locate("apps", service->entryPath()) );

        KPropertiesDialog dlg( desktopTemplate, KUrl(KGlobalSettings::autostartPath()), service->name() + ".desktop", this );
        if ( dlg.exec() != QDialog::Accepted )
            return;
    }
    Desktop * item = new Desktop( KGlobalSettings::autostartPath() + service->name() + ".desktop", widget->listCMD );
    addItem( item, service->name(), pathName.value(paths.indexOf((item->fileName().directory()+'/') )),  service->exec() );
    emit changed(true);
}

void Autostart::slotAddCMD() {
    AddDialog * addDialog = new AddDialog(this);
    int result = addDialog->exec();
    if (result == QDialog::Accepted) {
        if (addDialog->symLink())
            KIO::link(addDialog->importUrl(), paths[0]);
        else
            KIO::copy(addDialog->importUrl(), paths[0]);

        Desktop * item = new Desktop( paths[0] + addDialog->importUrl().fileName(), widget->listCMD );
        addItem( item,  addDialog->importUrl().fileName(), pathName.value(paths.indexOf((item->fileName().directory()+'/') ) ), addDialog->importUrl().fileName() );
    }
    delete addDialog;
    emit changed(true);
}

void Autostart::slotRemoveCMD() {
    QTreeWidgetItem* item = widget->listCMD->currentItem();
    if (!item) return;
    widget->listCMD->takeTopLevelItem( widget->listCMD->indexOfTopLevelItem( item ) );

    KIO::del(((Desktop*)item)->fileName().path() );
    emit changed(true);
}

void Autostart::slotEditCMD(QTreeWidgetItem* ent) {
    if (!ent) return;
    Desktop *entry = static_cast<Desktop*>( ent );

    const KFileItem kfi = KFileItem( KFileItem::Unknown, KFileItem::Unknown, KUrl( entry->fileName() ), true );
    if (! slotEditCMD( kfi )) return;

    if (entry->isDesktop()) {
        KService service(entry->fileName().path());
        addItem( entry, service.name(), pathName.value(paths.indexOf((entry->fileName().directory()+'/') )), service.exec() );
    }
}

bool Autostart::slotEditCMD( const KFileItem &item) {
    KPropertiesDialog dlg( item, this );
    bool c = ( dlg.exec() == QDialog::Accepted );
    emit changed(c);
    return c;
}

void Autostart::slotEditCMD() {
    if ( widget->listCMD->currentItem() == 0 )
        return;
    slotEditCMD( (Desktop*)widget->listCMD->currentItem() );
}

void Autostart::slotSetStartOn( int index ) {
    if ( widget->listCMD->currentItem() == 0 )
        return;
    Desktop* entry = (Desktop*)widget->listCMD->currentItem();
    entry->setPath(paths.value(index));
    entry->setText(COL_COMMAND, pathName[index]);
}

void Autostart::slotSelectionChanged() {
    bool hasItems = (widget->listCMD->currentItem()!= 0 );
    widget->cmbStartOn->setEnabled(hasItems);
    widget->btnRemove->setEnabled(hasItems);
    widget->btnProperties->setEnabled(hasItems);
    if (!hasItems)
        return;

    Desktop* entry = static_cast<Desktop*>( widget->listCMD->currentItem() );
    widget->cmbStartOn->setCurrentIndex( paths.indexOf(entry->fileName().directory()+'/') );
}

void Autostart::defaults()
{
}

void Autostart::save()
{
}

#include "autostart.moc"
