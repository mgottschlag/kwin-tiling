/*
 * shortcuts.cpp
 *
 * Copyright (c) 1999 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
 * Copyright (c) 2001 Ellis Whitehead <ellis@kde.org>
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
 *  Foundation, Inc., 51 Franklin Steet, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "shortcuts.h"

#include <qdir.h>
#include <qlayout.h>

//Added by qt3to4:
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QResizeEvent>
//Should have been added:
#include <Q3ButtonGroup>

#include <kapplication.h>
#include <kdebug.h>
#include <kipc.h>
#include <kkeynative.h>
#include <kinputdialog.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kshortcutlist.h>
#include <ksimpleconfig.h>
#include <kstandarddirs.h>

ShortcutsModule::ShortcutsModule( QWidget *parent, const char *name )
: QWidget( parent, name )
{
	initGUI();
}

ShortcutsModule::~ShortcutsModule()
{
	delete m_pListGeneral;
	delete m_pListSequence;
	delete m_pListApplication;
}

// Called when [Reset] is pressed
void ShortcutsModule::load()
{
	kdDebug(125) << "ShortcutsModule::load()" << endl;
	slotSchemeCur();
}

// When [Apply] or [OK] are clicked.
void ShortcutsModule::save()
{
	kdDebug(125) << "ShortcutsModule::save()" << endl;

	// FIXME: This isn't working.  Why? -- ellis, 2002/01/27
	// Check for old group,
	if( KGlobal::config()->hasGroup( "Keys" ) ) {
		KGlobal::config()->deleteGroup( "Keys", true, true );
	}
	KGlobal::config()->sync();

	m_pkcGeneral->commitChanges();
	m_pkcSequence->commitChanges();
	m_pkcApplication->save();

	m_actionsGeneral.writeActions( "Global Shortcuts", 0, true, true );
	m_actionsSequence.writeActions( "Global Shortcuts", 0, true, true );

	KIPC::sendMessageAll( KIPC::SettingsChanged, KApplication::SETTINGS_SHORTCUTS );
}

void ShortcutsModule::defaults()
{
	m_pkcGeneral->allDefault();
	m_pkcSequence->allDefault();
	m_pkcApplication->allDefault();
}

QString ShortcutsModule::quickHelp() const
{
  return i18n("<h1>Key Bindings</h1> Using key bindings you can configure certain actions to be"
    " triggered when you press a key or a combination of keys, e.g. Ctrl+C is normally bound to"
    " 'Copy'. KDE allows you to store more than one 'scheme' of key bindings, so you might want"
    " to experiment a little setting up your own scheme while you can still change back to the"
    " KDE defaults.<p> In the tab 'Global Shortcuts' you can configure non-application specific"
    " bindings like how to switch desktops or maximize a window. In the tab 'Application Shortcuts'"
    " you will find bindings typically used in applications, such as copy and paste.");
}

void ShortcutsModule::initGUI()
{
	kdDebug(125) << "A-----------" << endl;
	KAccelActions* keys = &m_actionsGeneral;
// see also KShortcutsModule::init() below !!!
#define NOSLOTS
#define KICKER_ALL_BINDINGS
#include "../../kwin/kwinbindings.cpp"
#include "../../../kicker/kicker/core/kickerbindings.cpp"
#include "../../../kicker/taskbar/taskbarbindings.cpp"
#include "../../../kdesktop/kdesktopbindings.cpp"
#include "../../../klipper/klipperbindings.cpp"
#include "../kxkb/kxkbbindings.cpp"

	kdDebug(125) << "B-----------" << endl;
	m_actionsSequence.init( m_actionsGeneral );

	kdDebug(125) << "C-----------" << endl;
	createActionsGeneral();
	kdDebug(125) << "D-----------" << endl;
	createActionsSequence();
	kdDebug(125) << "E-----------" << endl;

	kdDebug(125) << "F-----------" << endl;
	QVBoxLayout* pVLayout = new QVBoxLayout( this, KDialog::marginHint() );

	pVLayout->addSpacing( KDialog::marginHint() );

	// (o) [Current      ] <Remove>   ( ) New <Save>

	QHBoxLayout *pHLayout = new QHBoxLayout( pVLayout, KDialog::spacingHint() );
	Q3ButtonGroup* pGroup = new Q3ButtonGroup( this );
	pGroup->hide();

	m_prbPre = new QRadioButton( "", this );
	connect( m_prbPre, SIGNAL(clicked()), SLOT(slotSchemeCur()) );
	pGroup->insert( m_prbPre );
	pHLayout->addWidget( m_prbPre );

	m_pcbSchemes = new KComboBox( this );
	m_pcbSchemes->setMinimumWidth( 100 );
	m_pcbSchemes->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed );
	connect( m_pcbSchemes, SIGNAL(activated(int)), SLOT(slotSelectScheme(int)) );
	pHLayout->addWidget( m_pcbSchemes );

	pHLayout->addSpacing( KDialog::marginHint() );

	m_pbtnRemove = new QPushButton( i18n("&Remove"), this );
	m_pbtnRemove->setEnabled( false );
	connect( m_pbtnRemove, SIGNAL(clicked()), SLOT(slotRemoveScheme()) );
	m_pbtnRemove->setWhatsThis( i18n("Click here to remove the selected key bindings scheme. You cannot"
		" remove the standard system-wide schemes 'Current scheme' and 'KDE default'.") );
	pHLayout->addWidget( m_pbtnRemove );

	pHLayout->addSpacing( KDialog::marginHint() * 3 );

	m_prbNew = new QRadioButton( i18n("New scheme"), this );
	m_prbNew->setEnabled( false );
	pGroup->insert( m_prbNew );
	pHLayout->addWidget( m_prbNew );

	m_pbtnSave = new QPushButton( i18n("&Save..."), this );
	m_pbtnSave->setEnabled( false );
	m_pbtnSave->setWhatsThis( i18n("Click here to add a new key bindings scheme. You will be prompted for a name.") );
	connect( m_pbtnSave, SIGNAL(clicked()), SLOT(slotSaveSchemeAs()) );
	pHLayout->addWidget( m_pbtnSave );

	pHLayout->addStretch( 1 );

	m_pTab = new QTabWidget( this );
	m_pTab->setMargin( KDialog::marginHint() );
	pVLayout->addWidget( m_pTab );

	m_pListGeneral = new KAccelShortcutList( m_actionsGeneral, true );
	m_pkcGeneral = new KKeyChooser( m_pListGeneral, this, KKeyChooser::Global, false );
	m_pTab->addTab( m_pkcGeneral, i18n("&Global Shortcuts") );
	connect( m_pkcGeneral, SIGNAL(keyChange()), SLOT(slotKeyChange()) );

	m_pListSequence = new KAccelShortcutList( m_actionsSequence, true );
	m_pkcSequence = new KKeyChooser( m_pListSequence, this, KKeyChooser::Global, false );
	m_pTab->addTab( m_pkcSequence, i18n("Shortcut Se&quences") );
	connect( m_pkcSequence, SIGNAL(keyChange()), SLOT(slotKeyChange()) );

	m_pListApplication = new KStdAccel::ShortcutList;
	m_pkcApplication = new KKeyChooser( m_pListApplication, this, KKeyChooser::Standard, false );
	m_pTab->addTab( m_pkcApplication, i18n("App&lication Shortcuts") );
	connect( m_pkcApplication, SIGNAL(keyChange()), SLOT(slotKeyChange()) );

	kdDebug(125) << "G-----------" << endl;
	readSchemeNames();

	kdDebug(125) << "I-----------" << endl;
	slotSchemeCur();

	kdDebug(125) << "J-----------" << endl;
}

void ShortcutsModule::createActionsGeneral()
{
	KAccelActions& actions = m_actionsGeneral;

	for( uint i = 0; i < actions.count(); i++ ) {
		QString sConfigKey = actions[i].name();
		//kdDebug(125) << "sConfigKey: " << sConfigKey << endl;
		int iLastSpace = sConfigKey.findRev( ' ' );
		bool bIsNum = false;
		if( iLastSpace >= 0 )
			sConfigKey.mid( iLastSpace+1 ).toInt( &bIsNum );

		//kdDebug(125) << "sConfigKey: " << sConfigKey
		//	<< " bIsNum: " << bIsNum << endl;
		if( bIsNum && !sConfigKey.contains( ':' ) ) {
			actions[i].setConfigurable( false );
			actions[i].setName( QString::null );
		}
	}
}

void ShortcutsModule::createActionsSequence()
{
	KAccelActions& actions = m_actionsSequence;

	for( uint i = 0; i < actions.count(); i++ ) {
		QString sConfigKey = actions[i].name();
		//kdDebug(125) << "sConfigKey: " << sConfigKey << endl;
		int iLastSpace = sConfigKey.findRev( ' ' );
		bool bIsNum = false;
		if( iLastSpace >= 0 )
			sConfigKey.mid( iLastSpace+1 ).toInt( &bIsNum );

		//kdDebug(125) << "sConfigKey: " << sConfigKey
		//	<< " bIsNum: " << bIsNum << endl;
		if( !bIsNum && !sConfigKey.contains( ':' ) ) {
			actions[i].setConfigurable( false );
			actions[i].setName( QString::null );
		}
	}
}

void ShortcutsModule::readSchemeNames()
{
	QStringList schemes = KGlobal::dirs()->findAllResources("data", "kcmkeys/*.kksrc");

	m_pcbSchemes->clear();
	m_rgsSchemeFiles.clear();

	i18n("User-Defined Scheme");
	m_pcbSchemes->insertItem( i18n("Current Scheme") );
	m_rgsSchemeFiles.append( "cur" );

	// This for system files
	for ( QStringList::ConstIterator it = schemes.begin(); it != schemes.end(); ++it) {
	// KPersonalizer relies on .kksrc files containing all the keyboard shortcut
	//  schemes for various setups.  It also requires the KDE defaults to be in
	//  a .kksrc file.  The KDE defaults shouldn't be listed here.
		//if( r.search( *it ) != -1 )
		//   continue;

		KSimpleConfig config( *it, true );
		config.setGroup( "Settings" );
		QString str = config.readEntry( "Name" );

		m_pcbSchemes->insertItem( str );
		m_rgsSchemeFiles.append( *it );
	}
}

void ShortcutsModule::resizeEvent( QResizeEvent * )
{
	//m_pTab->setGeometry(0,0,width(),height());
}

void ShortcutsModule::slotSchemeCur()
{
	kdDebug(125) << "ShortcutsModule::slotSchemeCur()" << endl;
	//m_pcbSchemes->setCurrentItem( 0 );
	slotSelectScheme();
}

void ShortcutsModule::slotKeyChange()
{
	kdDebug(125) << "ShortcutsModule::slotKeyChange()" << endl;
	m_prbNew->setEnabled( true );
	m_prbNew->setChecked( true );
	m_pbtnSave->setEnabled( true );
	emit changed( true );
}

void ShortcutsModule::slotSelectScheme( int )
{
	i18n("Your current changes will be lost if you load another scheme before saving this one.");
	kdDebug(125) << "ShortcutsModule::slotSelectScheme( " << m_pcbSchemes->currentItem() << " )" << endl;
	QString sFilename = m_rgsSchemeFiles[ m_pcbSchemes->currentItem() ];

	if( sFilename == "cur" ) {
		// TODO: remove nulls params
		m_pkcGeneral->syncToConfig( "Global Shortcuts", 0, true );
		m_pkcSequence->syncToConfig( "Global Shortcuts", 0, true );
		m_pkcApplication->syncToConfig( "Shortcuts", 0, false );
	} else {
		KSimpleConfig config( sFilename );
		config.setGroup( "Settings" );
		//m_sBaseSchemeFile = config.readEntry( "Name" );

		// If the user's keyboard layout doesn't support the Win key,
		//  but this layout scheme requires it,
		if( !KKeyNative::keyboardHasWinKey()
		    && config.readBoolEntry( "Uses Win Modifier", false ) ) {
			// TODO: change "Win" to Win's label.
			int ret = KMessageBox::warningContinueCancel( this,
				i18n("This scheme requires the \"%1\" modifier key, which is not "
				"available on your keyboard layout. Do you wish to view it anyway?" )
				.arg(i18n("Win")) );
			if( ret == KMessageBox::Cancel )
				return;
		}

		m_pkcGeneral->syncToConfig( "Global Shortcuts", &config, true );
		m_pkcSequence->syncToConfig( "Global Shortcuts", &config, true );
		m_pkcApplication->syncToConfig( "Shortcuts", &config, false );
	}

	m_prbPre->setChecked( true );
	m_prbNew->setEnabled( false );
	m_pbtnSave->setEnabled( false );
	emit changed(true);
}

void ShortcutsModule::slotSaveSchemeAs()
{
	QString sName, sFile;
	bool bNameValid, ok;
	int iScheme = -1;

	sName = m_pcbSchemes->currentText();

	do {
		bNameValid = true;

		sName = KInputDialog::getText( i18n( "Save Key Scheme" ),
			i18n( "Enter a name for the key scheme:" ), sName, &ok, this );

		if( ok ) {
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
					i18n("A key scheme with the name '%1' already exists;\n"
						"do you want to overwrite it?\n").arg(sName),
					i18n("Save Key Scheme"),
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
		qWarning("KShortcutsModule: Could not make directory to store user info.");
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
	slotSelectScheme();
}

void ShortcutsModule::saveScheme()
{
	QString sFilename = m_rgsSchemeFiles[ m_pcbSchemes->currentItem() ];
	KSimpleConfig config( sFilename );

	m_pkcGeneral->commitChanges();
	m_pkcSequence->commitChanges();
	m_pkcApplication->commitChanges();

	m_pListGeneral->writeSettings( "Global Shortcuts", &config, true );
	m_pListSequence->writeSettings( "Global Shortcuts", &config, true );
	m_pListApplication->writeSettings( "Shortcuts", &config, true );
}

void ShortcutsModule::slotRemoveScheme()
{
}

#include "shortcuts.moc"
