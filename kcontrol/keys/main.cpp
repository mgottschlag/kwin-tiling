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

#include <qlayout.h>
#include <qregexp.h>
#include <klocale.h>
#include <kglobal.h>
#include <ksimpleconfig.h>
#include <kstandarddirs.h>
#include "keyconfig.h"
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
	QGridLayout* pLayout = new QGridLayout( this, 6, 3, KDialog::marginHint(), KDialog::spacingHint() );

	// What's QButtonGroup for?
	QButtonGroup* pGroup = new QButtonGroup( this );
	m_prbCur = new QRadioButton( i18n("Current scheme"), this );
	m_prbNew = new QRadioButton( i18n("New scheme"), this );
	m_prbPre = new QRadioButton( i18n("Preset scheme"), this );
	pGroup->setExclusive( true );
	pGroup->insert( m_prbCur );
	pGroup->insert( m_prbNew );
	pGroup->insert( m_prbPre );
	pLayout->addWidget( m_prbCur, 1, 0 );
	pLayout->addWidget( m_prbNew, 2, 0 );
	pLayout->addWidget( m_prbPre, 3, 0 );

	m_pcbSchemes = new KComboBox( this );
	pLayout->addMultiCellWidget( m_pcbSchemes, 4, 4, 0, 2 );
	//QWhatsThis::add( rb, i18n("The selected action will not be associated with any key.") );

	m_pTab = new QTabWidget( this );
	pLayout->addMultiCellWidget( m_pTab, 5, 5, 0, 2 );
	connect(m_pTab, SIGNAL(currentChanged(QWidget*)), SLOT(tabChanged(QWidget*)));

	global = new KKeyModule(this, true, false, true);
	m_pTab->addTab(global, i18n("&Global Shortcuts"));
	connect(global, SIGNAL(changed(bool)), this, SLOT(moduleChanged(bool)));

	series = new KKeyModule(this, true, true, false);
	m_pTab->addTab(series, i18n("Shortcut Se&quences"));
	connect(series, SIGNAL(changed(bool)), this, SLOT(moduleChanged(bool)));

	standard = new KKeyModule(this, false);
	m_pTab->addTab(standard, i18n("&Application Shortcuts"));
	connect(standard, SIGNAL(changed(bool)), this, SLOT(moduleChanged(bool)));

	connect(standard, SIGNAL(keysChanged( const KKeyEntryMap* )),
		global, SLOT( updateKeys( const KKeyEntryMap* )));
	connect(global, SIGNAL(keysChanged( const KKeyEntryMap* )),
		standard, SLOT( updateKeys( const KKeyEntryMap* )));
}

void KeyModule::readSchemeNames()
{
	//QStringList schemes = KGlobal::dirs()->findAllResources("data", "kcmkeys/" + KeyType + "/*.kksrc");
	QStringList schemes = KGlobal::dirs()->findAllResources("data", "kcmkeys/" "global" "/*.kksrc");
	//QRegExp r( "-kde[34].kksrc$" );
	QRegExp r( "-kde3.kksrc$" );

	m_pcbSchemes->clear();
	m_rgsSchemeFiles.clear();
	m_pcbSchemes->insertItem( i18n("Current Scheme"), 0 );
	m_pcbSchemes->insertItem( i18n("KDE Traditional"), 1 );
	//m_pcbSchemes->insertItem( i18n("KDE Extended (With 'Win' Key)"), 2 );
	//m_pcbSchemes->insertItem( i18n("KDE Default for 4 Modifiers (Meta/Alt/Ctrl/Shift)"), 2 );
	//sFileList->append( "Not a kcsrc file" );
	m_nSysSchemes = 2;

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
		config.setGroup( "Global Key Scheme" );
		QString str = config.readEntry( "Name" );

		m_pcbSchemes->insertItem( str );
		m_rgsSchemeFiles.append( *it );
	}
}

void KeyModule::load()
{
	global->load();
	series->load();
	standard->load();
}

void KeyModule::save()
{
	global->save();
	series->save();
	standard->save();
}

void KeyModule::defaults()
{
	global->defaults();
	series->defaults();
	standard->defaults();
}

void KeyModule::moduleChanged( bool bState )
{
	emit changed( bState );
}

// Keep the global and sequence shortcut scheme lists in sync
void KeyModule::tabChanged(QWidget*)
{
  global->readSchemeNames();
  series->readSchemeNames();
}

void KeyModule::resizeEvent(QResizeEvent *)
{
  m_pTab->setGeometry(0,0,width(),height());
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
