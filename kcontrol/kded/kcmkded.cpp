/* This file is part of the KDE project
   Copyright (C) 2002 Daniel Molkentin <molkentin@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include <q3groupbox.h>
#include <q3header.h>

#include <QByteArray>
#include <dbus/qdbus.h>
#include <QLayout>
#include <QPushButton>
#include <QTimer>
#include <QVBoxLayout>

#include <kaboutdata.h>
#include <kapplication.h>
#include <kbuttonbox.h>
#include <kdebug.h>
#include <kdesktopfile.h>
#include <kdialog.h>
#include <kgenericfactory.h>
#include <k3listview.h>
#include <kmessagebox.h>
#include <kservice.h>
#include <kstandarddirs.h>

#include "kcmkded.h"
#include "kcmkded.moc"

typedef KGenericFactory<KDEDConfig, QWidget> KDEDFactory;
K_EXPORT_COMPONENT_FACTORY( kcm_kded, KDEDFactory( "kcmkded" ) )


KDEDConfig::KDEDConfig(QWidget* parent, const QStringList &) :
	KCModule( KDEDFactory::instance(), parent )
{
	KAboutData *about =
		new KAboutData( I18N_NOOP( "kcmkded" ), I18N_NOOP( "KDE Service Manager" ),
				0, 0, KAboutData::License_GPL,
				I18N_NOOP( "(c) 2002 Daniel Molkentin" ) );
	about->addAuthor("Daniel Molkentin",0,"molkentin@kde.org");
	setAboutData( about );

	setQuickHelp( i18n("<h1>Service Manager</h1><p>This module allows you to have an overview of all plugins of the "
			"KDE Daemon, also referred to as KDE Services. Generally, there are two types of service:</p>"
			"<ul><li>Services invoked at startup</li><li>Services called on demand</li></ul>"
			"<p>The latter are only listed for convenience. The startup services can be started and stopped. "
			"In Administrator mode, you can also define whether services should be loaded at startup.</p>"
			"<p><b> Use this with care: some services are vital for KDE; do not deactivate services if you"
			" do not know what you are doing.</b></p>"));

	RUNNING = i18n("Running")+' ';
	NOT_RUNNING = i18n("Not running")+' ';

	QVBoxLayout *lay = new QVBoxLayout( this );
	lay->setMargin( 0 );
	lay->setSpacing( KDialog::spacingHint() );

	Q3GroupBox *gb = new Q3GroupBox(1, Qt::Vertical, i18n("Load-on-Demand Services"), this );
	gb->setWhatsThis( i18n("This is a list of available KDE services which will "
			"be started on demand. They are only listed for convenience, as you "
			"cannot manipulate these services."));
	lay->addWidget( gb );

	_lvLoD = new K3ListView( gb );
	_lvLoD->addColumn(i18n("Service"));
	_lvLoD->addColumn(i18n("Description"));
	_lvLoD->addColumn(i18n("Status"));
	_lvLoD->setAllColumnsShowFocus(true);
	_lvLoD->header()->setStretchEnabled(true, 1);

 	gb = new Q3GroupBox(1, Qt::Horizontal, i18n("Startup Services"), this );
	gb->setWhatsThis( i18n("This shows all KDE services that can be loaded "
				"on KDE startup. Checked services will be invoked on next startup. "
				"Be careful with deactivation of unknown services."));
	lay->addWidget( gb );

	_lvStartup = new K3ListView( gb );
	_lvStartup->addColumn(i18n("Use"));
	_lvStartup->addColumn(i18n("Service"));
	_lvStartup->addColumn(i18n("Description"));
	_lvStartup->addColumn(i18n("Status"));
	_lvStartup->setAllColumnsShowFocus(true);
	_lvStartup->header()->setStretchEnabled(true, 2);

	KButtonBox *buttonBox = new KButtonBox( gb, Qt::Horizontal);
	_pbStart = buttonBox->addButton( i18n("Start"));
	_pbStop = buttonBox->addButton( i18n("Stop"));

	_pbStart->setEnabled( false );
	_pbStop->setEnabled( false );

	connect(_pbStart, SIGNAL(clicked()), SLOT(slotStartService()));
	connect(_pbStop, SIGNAL(clicked()), SLOT(slotStopService()));
	connect(_lvStartup, SIGNAL(selectionChanged(Q3ListViewItem*)), SLOT(slotEvalItem(Q3ListViewItem*)) );

	load();
}

void setModuleGroup(KConfig *config, const QString &filename)
{
	QString module = filename;
	int i = module.lastIndexOf('/');
	if (i != -1)
	   module = module.mid(i+1);
	i = module.lastIndexOf('.');
	if (i != -1)
	   module = module.left(i);

	config->setGroup(QString("Module-%1").arg(module));
}

bool KDEDConfig::autoloadEnabled(KConfig *config, const QString &filename)
{
	setModuleGroup(config, filename);
	return config->readEntry("autoload", true);
}

void KDEDConfig::setAutoloadEnabled(KConfig *config, const QString &filename, bool b)
{
	setModuleGroup(config, filename);
	return config->writeEntry("autoload", b);
}

void KDEDConfig::load() {
	KConfig kdedrc("kdedrc", true, false);

	_lvStartup->clear();
	_lvLoD->clear();

	QStringList files;
	KGlobal::dirs()->findAllResources( "services",
			QLatin1String( "kded/*.desktop" ),
			true, true, files );

	Q3ListViewItem* item = 0L;
	CheckListItem* clitem;
	for ( QStringList::ConstIterator it = files.begin(); it != files.end(); ++it ) {

		if ( KDesktopFile::isDesktopFile( *it ) ) {
			KDesktopFile file( *it, true, "services" );

			if ( file.readEntry("X-KDE-Kded-autoload", false) ) {
				clitem = new CheckListItem(_lvStartup, QString());
				connect(clitem, SIGNAL(changed(Q3CheckListItem*)), SLOT(slotItemChecked(Q3CheckListItem*)));
				clitem->setOn(autoloadEnabled(&kdedrc, *it));
				item = clitem;
				item->setText(1, file.readName());
				item->setText(2, file.readComment());
				item->setText(3, NOT_RUNNING);
				item->setText(4, file.readEntry("X-KDE-Library"));
			}
			else if ( file.readEntry("X-KDE-Kded-load-on-demand", false) ) {
				item = new Q3ListViewItem(_lvLoD, file.readName());
				item->setText(1, file.readComment());
				item->setText(2, NOT_RUNNING);
				item->setText(4, file.readEntry("X-KDE-Library"));
			}
		}
	}

	getServiceStatus();
}

void KDEDConfig::save() {
	Q3CheckListItem* item = 0L;

	QStringList files;
	KGlobal::dirs()->findAllResources( "services",
			QLatin1String( "kded/*.desktop" ),
			true, true, files );

	KConfig kdedrc("kdedrc", false, false);

	for ( QStringList::ConstIterator it = files.begin(); it != files.end(); ++it ) {

		if ( KDesktopFile::isDesktopFile( *it ) ) {

			KConfig file( *it, false, false, "services" );
			file.setGroup("Desktop Entry");

			if (file.readEntry("X-KDE-Kded-autoload", false)){

				item = static_cast<Q3CheckListItem *>(_lvStartup->findItem(file.readEntry("X-KDE-Library"),4));
				if (item) {
					// we found a match, now compare and see what changed
					setAutoloadEnabled(&kdedrc, *it, item->isOn());
				}
			}
		}
	}
	kdedrc.sync();

	QDBusInterfacePtr kdedInterface( "org.kde.kded", "/kded" );
	kdedInterface->call( "reconfigure" );
	QTimer::singleShot(0, this, SLOT(slotServiceRunningToggled()));
}


void KDEDConfig::defaults()
{
	Q3ListViewItemIterator it( _lvStartup);
	while ( it.current() != 0 ) {
		if (it.current()->rtti()==1) {
			Q3CheckListItem *item = static_cast<Q3CheckListItem *>(it.current());
			item->setOn(false);
		}
		++it;
	}

	getServiceStatus();
}


void KDEDConfig::getServiceStatus()
{
	QStringList modules;
	QDBusInterfacePtr kdedInterface( "org.kde.kded", "/kded" );
	QDBusReply<QStringList> reply = kdedInterface->call( "loadedModules"  );

	if ( reply.isSuccess() ) {
		modules = reply.value();
	}
	else {
		_lvLoD->setEnabled( false );
		_lvStartup->setEnabled( false );
		KMessageBox::error(this, i18n("Unable to contact KDED."));
		return;
	}

	for( Q3ListViewItemIterator it( _lvLoD); it.current() != 0; ++it )
                it.current()->setText(2, NOT_RUNNING);
	for( Q3ListViewItemIterator it( _lvStartup); it.current() != 0; ++it )
                it.current()->setText(3, NOT_RUNNING);
	foreach( const QString& module, modules )
	{
		Q3ListViewItem *item = _lvLoD->findItem(module, 4);
		if ( item )
		{
			item->setText(2, RUNNING);
		}

		item = _lvStartup->findItem(module, 4);
		if ( item )
		{
			item->setText(3, RUNNING);
		}
	}
}

void KDEDConfig::slotReload()
{
	QString current = _lvStartup->currentItem()->text(4);
	load();
	Q3ListViewItem *item = _lvStartup->findItem(current, 4);
	if (item)
		_lvStartup->setCurrentItem(item);
}

void KDEDConfig::slotEvalItem(Q3ListViewItem * item)
{
	if (!item)
		return;

	if ( item->text(3) == RUNNING ) {
		_pbStart->setEnabled( false );
		_pbStop->setEnabled( true );
	}
	else if ( item->text(3) == NOT_RUNNING ) {
		_pbStart->setEnabled( true );
		_pbStop->setEnabled( false );
	}
	else // Error handling, better do nothing
	{
		_pbStart->setEnabled( false );
		_pbStop->setEnabled( false );
	}

	getServiceStatus();
}

void KDEDConfig::slotServiceRunningToggled()
{
	getServiceStatus();
	slotEvalItem(_lvStartup->currentItem());
}

void KDEDConfig::slotStartService()
{
	QString service = _lvStartup->currentItem()->text(4).toLatin1();

	QDBusInterfacePtr kdedInterface( "org.kde.kded", "/kded" );
	QDBusReply<bool> reply = kdedInterface->call( "loadModule", service  );

	if ( reply.isSuccess() ) {
		if ( reply.value() )
			slotServiceRunningToggled();
		else
			KMessageBox::error(this, "<qt>" + i18n("Unable to start server <em>service</em>.") + "</qt>");
	}
	else {
		KMessageBox::error(this, "<qt>" + i18n("Unable to start service <em>service</em>.<br><br><i>Error: %1</i>",
											reply.error().message()) + "</qt>" );
	}
}

void KDEDConfig::slotStopService()
{
	QString service = _lvStartup->currentItem()->text(4).toLatin1();
	kDebug() << "Stopping: " << service << endl;

	QDBusInterfacePtr kdedInterface( "org.kde.kded", "/kded" );
	QDBusReply<bool> reply = kdedInterface->call( "unloadModule", service  );

	if ( reply.isSuccess() ) {
		if ( reply.value() )
			slotServiceRunningToggled();
		else
			KMessageBox::error(this, "<qt>" + i18n("Unable to stop server <em>service</em>.") + "</qt>");
	}
	else {
		KMessageBox::error(this, "<qt>" + i18n("Unable to stop service <em>service</em>.<br><br><i>Error: %1</i>",
											reply.error().message()) + "</qt>" );
	}
}

void KDEDConfig::slotItemChecked(Q3CheckListItem*)
{
	emit changed(true);
}

CheckListItem::CheckListItem(Q3ListView *parent, const QString &text)
	: QObject(parent),
	  Q3CheckListItem(parent, text, CheckBox)
{ }

void CheckListItem::stateChange(bool on)
{
	Q3CheckListItem::stateChange(on);
	emit changed(this);
}
