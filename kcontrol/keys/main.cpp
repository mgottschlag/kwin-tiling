/*
 * main.cpp
 *
 * Copyright (c) 1999 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
 *
 * Requires the Qt widget libraries, available at no cost at
 * http://www.troll.no/
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "main.h"
#include "keyconfig.h"

#include <qdir.h>
#include <qlayout.h>
#include <qhbuttongroup.h>
#include <qregexp.h>
#include <qwhatsthis.h>

#include <dcopclient.h>
#include <kapplication.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kipc.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <ksimpleconfig.h>
#include <kstandarddirs.h>
/*
New Scheme
Current Scheme
System Schemes
User Defined Schemes
- Save
- Remove Scheme

o Current scheme  o New scheme  o Pre-set scheme
| KDE Traditional |v|| <Save Scheme...> <Remove Scheme>
[] Prefer 4-modifier defaults

o Current scheme
o New scheme       <Save Scheme>
o Pre-set scheme   <Remove Scheme>
  | KDE Traditional |v||
[] Prefer 4-modifier defaults

Global Shortcuts
*/
KeyModule::KeyModule( QWidget *parent, const char *name )
: KCModule( parent, name )
{
	init();
}

void KeyModule::load()
{
	kdDebug() << "KeyModule::load()" << endl;
	//global->load();
	//series->load();
	//standard->load();
}

// When [Apply] or [OK] are clicked.
void KeyModule::save()
{
	kdDebug() << "KeyModule::save()" << endl;

	//global->writeSettingsGlobal( "Global Shortcuts" );
	m_pkcGeneral->commitChanges();
	m_pkcSequence->commitChanges();
	m_pkcApplication->commitChanges();

	m_actionsGeneral.writeActions( "Global Shortcuts", 0, true, true );
	m_actionsSequence.writeActions( "Global Shortcuts", 0, true, true );
	m_actionsApplication.writeActions( "Shortcuts", 0, true, true );

	// TODO: use KIPC to send a reconfigure message!
	if ( !kapp->dcopClient()->isAttached() )
		kapp->dcopClient()->attach();
	// TODO: create a reconfigureKeys() method.
	//kapp->dcopClient()->send("kwin", "", "reconfigure()", "");
	kapp->dcopClient()->send("kdesktop", "", "configure()", "");
	//kapp->dcopClient()->send("kicker", "Panel", "configure()", "");

	KIPC::sendMessageAll( KIPC::SettingsChanged, KApplication::SETTINGS_SHORTCUTS );
}

void KeyModule::defaults()
{
	m_pkcGeneral->allDefault();
	m_pkcSequence->allDefault();
	m_pkcApplication->allDefault();
}

QString KeyModule::quickHelp() const
{
  return i18n("<h1>Key Bindings</h1> Using key bindings you can configure certain actions to be"
    " triggered when you press a key or a combination of keys, e.g. CTRL-C is normally bound to"
    " 'Copy'. KDE allows you to store more than one 'scheme' of key bindings, so you might want"
    " to experiment a little setting up your own scheme while you can still change back to the"
    " KDE defaults.<p> In the tab 'Global shortcuts' you can configure non-application specific"
    " bindings like how to switch desktops or maximize a window. In the tab 'Application shortcuts'"
    " you'll find bindings typically used in applications, such as copy and paste.");
}

void KeyModule::init()
{
	kdDebug(125) << "A-----------" << endl;
	KAccelActions* keys = &m_actionsGeneral;
// see also KKeyModule::init() below !!!
#define NOSLOTS
#define KShortcuts KAccelShortcuts
#define KICKER_ALL_BINDINGS
#include "../../kwin/kwinbindings.cpp"
#include "../../kicker/core/kickerbindings.cpp"
#include "../../kdesktop/kdesktopbindings.cpp"
#include "../../klipper/klipperbindings.cpp"
#include "../../kxkb/kxkbbindings.cpp"
#undef KShortcuts

	kdDebug(125) << "B-----------" << endl;
	m_actionsSequence.init( m_actionsGeneral );

	kdDebug(125) << "C-----------" << endl;
	createActionsGeneral();
	kdDebug(125) << "D-----------" << endl;
	createActionsSequence();
	kdDebug(125) << "E-----------" << endl;
	createActionsApplication();

	kdDebug(125) << "F-----------" << endl;
	QGridLayout* pLayout = new QGridLayout( this, 6, 3, KDialog::marginHint(), KDialog::spacingHint() );

	// What's QButtonGroup for?
	QButtonGroup* pGroup = new QButtonGroup( this );
	pGroup->hide();

	m_prbCur = new QRadioButton( i18n("Current scheme"), this );
	pGroup->insert( m_prbCur );
	pLayout->addWidget( m_prbCur, 1, 0 );
	connect( m_prbCur, SIGNAL(clicked()), SLOT(slotSchemeCur()) );

	m_prbNew = new QRadioButton( i18n("New scheme"), this );
	m_prbNew->setEnabled( false );
	pGroup->insert( m_prbNew );
	pLayout->addWidget( m_prbNew, 1, 1 );

	m_prbPre = new QRadioButton( i18n("Preset scheme"), this );
	m_prbPre->setEnabled( false );
	pGroup->insert( m_prbPre );
	pLayout->addWidget( m_prbPre, 1, 2 );

	m_pcbSchemes = new KComboBox( this );
	pLayout->addMultiCellWidget( m_pcbSchemes, 2, 2, 0, 2 );
	//QWhatsThis::add( rb, i18n("The selected action will not be associated with any key.") );
	connect( m_pcbSchemes, SIGNAL(activated(int)), SLOT(slotSelectScheme(int)) );

	m_pbtnSave = new QPushButton( i18n("&Save Scheme..."), this );
	m_pbtnSave->setEnabled( false );
	QWhatsThis::add( m_pbtnSave, i18n("Click here to add a new key bindings scheme. You will be prompted for a name.") );
	pLayout->addWidget( m_pbtnSave, 3, 0 );
	connect( m_pbtnSave, SIGNAL(clicked()), SLOT(slotSaveSchemeAs()) );

	m_pbtnRemove = new QPushButton( i18n("&Remove Scheme"), this );
	m_pbtnRemove->setEnabled( false );
	connect( m_pbtnRemove, SIGNAL(clicked()), SLOT(slotRemoveScheme()) );
	QWhatsThis::add( m_pbtnRemove, i18n("Click here to remove the selected key bindings scheme. You can not"
		" remove the standard system wide schemes, 'Current scheme' and 'KDE default'.") );
	pLayout->addWidget( m_pbtnRemove, 3, 1 );

	m_pTab = new QTabWidget( this );
	pLayout->addMultiCellWidget( m_pTab, 5, 5, 0, 2 );
	connect(m_pTab, SIGNAL(currentChanged(QWidget*)), SLOT(tabChanged(QWidget*)));

	m_pkcGeneral = new KKeyChooser( m_actionsGeneral, this, true, false, true );
	m_pTab->addTab( m_pkcGeneral, i18n("&Global Shortcuts") );
	connect( m_pkcGeneral, SIGNAL(keyChange()), SLOT(slotKeyChange()) );

	m_pkcSequence = new KKeyChooser( m_actionsSequence, this, true, false, true );
	m_pTab->addTab( m_pkcSequence, i18n("Shortcut Se&quences") );
	connect( m_pkcGeneral, SIGNAL(keyChange()), SLOT(slotKeyChange()) );

	m_pkcApplication = new KKeyChooser( m_actionsApplication, this, true, false, false );
	m_pTab->addTab( m_pkcApplication, i18n("&Application Shortcuts") );
	connect( m_pkcApplication, SIGNAL(keyChange()), SLOT(slotKeyChange()) );

	kdDebug(125) << "G-----------" << endl;
	readSchemeNames();

	kdDebug(125) << "I-----------" << endl;
	slotSchemeCur();

	kdDebug(125) << "J-----------" << endl;
}

void KeyModule::createActionsGeneral()
{
	KAccelActions& actions = m_actionsGeneral;

	for( uint i = 0; i < actions.size(); i++ ) {
		QString sConfigKey = actions[i].m_sName;
		//kdDebug(125) << "sConfigKey: " << sConfigKey << endl;
		int iLastSpace = sConfigKey.findRev( ' ' );
		bool bIsNum = false;
		if( iLastSpace >= 0 )
			sConfigKey.mid( iLastSpace+1 ).toInt( &bIsNum );

		//kdDebug(125) << "sConfigKey: " << sConfigKey
		//	<< " bIsNum: " << bIsNum << endl;
		if( bIsNum && !sConfigKey.contains( ':' ) ) {
			actions[i].m_bConfigurable = false;
			actions[i].m_sName = QString::null;
		}
	}
}

void KeyModule::createActionsSequence()
{
	KAccelActions& actions = m_actionsSequence;

	for( uint i = 0; i < actions.size(); i++ ) {
		QString sConfigKey = actions[i].m_sName;
		//kdDebug(125) << "sConfigKey: " << sConfigKey << endl;
		int iLastSpace = sConfigKey.findRev( ' ' );
		bool bIsNum = false;
		if( iLastSpace >= 0 )
			sConfigKey.mid( iLastSpace+1 ).toInt( &bIsNum );

		//kdDebug(125) << "sConfigKey: " << sConfigKey
		//	<< " bIsNum: " << bIsNum << endl;
		if( !bIsNum && !sConfigKey.contains( ':' ) ) {
			actions[i].m_bConfigurable = false;
			actions[i].m_sName = QString::null;
		}
	}
}

void KeyModule::createActionsApplication()
{
	for( uint i=0; i < KStdAccel::NB_STD_ACCELS; i++ ) {
		KStdAccel::StdAccel id = (KStdAccel::StdAccel) i;
		m_actionsApplication.insertAction( KStdAccel::action(id),
			KStdAccel::description(id),
			KStdAccel::defaultKey3(id),
			KStdAccel::defaultKey4(id) );
	}
}

void KeyModule::readSchemeNames()
{
	//QStringList schemes = KGlobal::dirs()->findAllResources("data", "kcmkeys/" + KeyType + "/*.kksrc");
	QStringList schemes = KGlobal::dirs()->findAllResources("data", "kcmkeys/*.kksrc");
	//QRegExp r( "-kde[34].kksrc$" );
	//QRegExp r( "-kde3.kksrc$" );

	m_pcbSchemes->clear();
	m_rgsSchemeFiles.clear();

	m_pcbSchemes->insertItem( "Current Scheme" );
	m_rgsSchemeFiles.append( "cur" );

	// This for system files
	for ( QStringList::ConstIterator it = schemes.begin(); it != schemes.end(); it++) {
	// KPersonalizer relies on .kksrc files containing all the keyboard shortcut
	//  schemes for various setups.  It also requires the KDE defaults to be in
	//  a .kksrc file.  The KDE defaults shouldn't be listed here.
		//if( r.search( *it ) != -1 )
		//   continue;

		KSimpleConfig config( *it, true );
		// TODO: Put 'Name' in "Settings" group
		//config.setGroup( KeyScheme );
		config.setGroup( "Settings" );
		QString str = config.readEntry( "Name" );

		m_pcbSchemes->insertItem( str );
		m_rgsSchemeFiles.append( *it );
	}
}

void KeyModule::resizeEvent( QResizeEvent * )
{
	m_pTab->setGeometry(0,0,width(),height());
}

void KeyModule::slotSchemeCur()
{
	kdDebug() << "KeyModule::slotSchemeCur()" << endl;
	m_pcbSchemes->setCurrentItem( 0 );
	slotSelectScheme( 0 );
}

void KeyModule::slotKeyChange()
{
	kdDebug() << "KeyModule::slotKeyChange()" << endl;
	m_prbNew->setEnabled( true );
	m_prbNew->setChecked( true );
	m_pbtnSave->setEnabled( true );
}

void KeyModule::slotSelectScheme( int )
{
	kdDebug() << "KeyModule::slotSelectScheme( " << m_pcbSchemes->currentItem() << " )" << endl;
	QString sFilename = m_rgsSchemeFiles[ m_pcbSchemes->currentItem() ];

	if( sFilename == "cur" ) {
		m_actionsGeneral.readActions( "Global Shortcuts", 0 );
		m_actionsSequence.readActions( "Global Shortcuts", 0 );
		m_actionsApplication.readActions( "Shortcuts", 0 );

		m_prbCur->setChecked( true );
		m_prbNew->setEnabled( false );
		m_pbtnSave->setEnabled( false );
	} else {
		KSimpleConfig config( sFilename );

		m_actionsGeneral.readActions( "Global Shortcuts", &config );
		m_actionsSequence.readActions( "Global Shortcuts", &config );
		m_actionsApplication.readActions( "Shortcuts", &config );

		m_prbNew->setEnabled( false );
		m_prbPre->setEnabled( true );
		m_prbPre->setChecked( true );
		m_pbtnSave->setEnabled( false );
	}

	m_pkcGeneral->listSync();
	m_pkcSequence->listSync();
	m_pkcApplication->listSync();
}

void KeyModule::slotSaveSchemeAs()
{
	QString sName, sFile;
	bool bNameValid;
	int iScheme = -1;

	sName = m_pcbSchemes->currentText();

	SaveScm ss( 0, "save scheme", sName );
	do {
		bNameValid = true;

		if( ss.exec() ) {
			sName = ss.nameLine->text();
			if( sName.stripWhiteSpace().isEmpty() )
				return;

			sName = sName.simplifyWhiteSpace();
			sFile = sName;

			int ind = 0;
			while( ind < (int) sFile.length() ) {
				// parse the string for first white space
				ind = sFile.find(" ");
				if( ind == -1 ) {
					ind = sFile.length();
					break;
				}

				// remove from string
				sFile.remove( ind, 1 );

				// Make the next letter upper case
				QString s = sFile.mid( ind, 1 );
				s = s.upper();
				sFile.replace( ind, 1, s );
			}

			iScheme = -1;
			for( int i = 0; i < (int) m_pcbSchemes->count(); i++ ) {
				if( sName.lower() == (m_pcbSchemes->text(i)).lower() ) {
					iScheme = i;

					int result = KMessageBox::warningContinueCancel( 0,
					i18n("A key scheme with the name '%1' already exists.\n"
						"Do you want to overwrite it?\n").arg(sName),
					i18n("Save key scheme"),
					i18n("Overwrite"));
					bNameValid = (result == KMessageBox::Continue);
				}
			}
		} else
			return;
	} while( !bNameValid );

	disconnect( m_pcbSchemes, SIGNAL(activated(int)), this, SLOT(slotSelectScheme(int)) );

	QString kksPath = KGlobal::dirs()->saveLocation( "data", "kcmkeys/" );

	QDir dir( kksPath );
	if( !dir.exists() && !dir.mkdir( kksPath ) ) {
		qWarning("KKeyModule: Could not make directory to store user info.");
		return;
	}

	sFile.prepend( kksPath );
	sFile += ".kksrc";
	if( iScheme == -1 ) {
		m_pcbSchemes->insertItem( sName );
		//m_pcbSchemes->setFocus();
		m_pcbSchemes->setCurrentItem( m_pcbSchemes->count()-1 );
		m_rgsSchemeFiles.append( sFile );
	} else {
		//m_pcbSchemes->setFocus();
		m_pcbSchemes->setCurrentItem( iScheme );
	}

	KSimpleConfig *config = new KSimpleConfig( sFile );

	config->setGroup( "Settings" );
	config->writeEntry( "Name", sName );
	delete config;

	saveScheme();

	connect( m_pcbSchemes, SIGNAL(activated(int)), SLOT(slotSelectScheme(int)) );
	slotSelectScheme( m_pcbSchemes->currentItem() );
}

void KeyModule::saveScheme()
{
	QString sFilename = m_rgsSchemeFiles[ m_pcbSchemes->currentItem() ];
	KSimpleConfig config( sFilename );

	m_pkcGeneral->commitChanges();
	m_pkcSequence->commitChanges();
	m_pkcApplication->commitChanges();

	m_actionsGeneral.writeActions( "Global Shortcuts", &config, true );
	m_actionsSequence.writeActions( "Global Shortcuts", &config, true );
	m_actionsApplication.writeActions( "Shortcuts", &config, true );
}

void KeyModule::slotRemoveScheme()
{
}

void KeyModule::moduleChanged( bool bState )
{
	emit changed( bState );
}

// Keep the global and sequence shortcut scheme lists in sync
void KeyModule::tabChanged( QWidget* )
{
	//global->readSchemeNames();
	//series->readSchemeNames();
}

//----------------------------------------------------

extern "C"
{
  KCModule *create_keys(QWidget *parent, const char *name)
  {
    KGlobal::locale()->insertCatalogue("kcmkeys");
    KGlobal::locale()->insertCatalogue("kwin");
    KGlobal::locale()->insertCatalogue("kdesktop");
    KGlobal::locale()->insertCatalogue("kicker");
    return new KeyModule(parent, name);
  }

  void init_keys()
  {
    KKeyModule::init();
  }
}

#include "main.moc"
