/*
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <QLayout>
//Added by qt3to4:
#include <QVBoxLayout>
#include <QResizeEvent>

#include <kdebug.h>
#include <klocale.h>
#include <ksimpleconfig.h>
#include <kinstance.h>
#include <kaction.h>
#include <kactioncollection.h>
#include <kglobalaccel.h>
#include <kgenericfactory.h>

#include "main.h"
#include "shortcuts.h"
#include "khotkeys.h"

typedef KGenericFactory<KeyModule> KeyModuleFactory;
K_EXPORT_COMPONENT_FACTORY(keys, KeyModuleFactory("kcmkeys"))

/*
| Shortcut Schemes | Modifier Keys |

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
KeyModule::KeyModule(QWidget *parent, const QStringList& args)
    : KCModule(KeyModuleFactory::instance(), parent, args)
{
    setQuickHelp( i18n("<h1>Keyboard Shortcuts</h1> Using shortcuts you can configure certain actions to be"
    " triggered when you press a key or a combination of keys, e.g. Ctrl+C is normally bound to"
    " 'Copy'. KDE allows you to store more than one 'scheme' of shortcuts, so you might want"
    " to experiment a little setting up your own scheme, although you can still change back to the"
    " KDE defaults.<p> In the 'Global Shortcuts' tab you can configure non-application-specific"
    " bindings, like how to switch desktops or maximize a window; in the 'Application Shortcuts' tab"
    " you will find bindings typically used in applications, such as copy and paste."));

	initGUI();
}

KeyModule::~KeyModule()
{
    KHotKeys::cleanup();
}

void KeyModule::initGUI()
{
	m_pTab = new QTabWidget( this );
	QVBoxLayout *l = new QVBoxLayout(this);
	l->addWidget(m_pTab);

	m_pShortcuts = new ShortcutsModule( this );
	m_pTab->addTab( m_pShortcuts, i18n("Shortcut Schemes") );
	connect( m_pShortcuts, SIGNAL(changed(bool)), SIGNAL(changed(bool)) );
}

// Called when [Reset] is pressed
void KeyModule::load()
{
	kDebug(125) << "KeyModule::load()" << endl;
	m_pShortcuts->load();
}

// When [Apply] or [OK] are clicked.
void KeyModule::save()
{
	kDebug(125) << "KeyModule::save()" << endl;
	m_pShortcuts->save();
}

void KeyModule::defaults()
{
	kDebug(125) << "KeyModule::defaults()" << endl;
	m_pShortcuts->defaults();
}

void KeyModule::resizeEvent( QResizeEvent * )
{
	m_pTab->setGeometry( 0, 0, width(), height() );
}

//----------------------------------------------------

extern "C"
{
  KDE_EXPORT void kcminit_keys()
  {
	kDebug(125) << "KeyModule::init()\n";

	kDebug(125) << "KeyModule::init() - Load Included Bindings\n";

	KActionCollection* actionCollection = new KActionCollection(static_cast<QObject*>(0L));
	QAction* a = 0L;
// this should match the included files above
#define NOSLOTS
#include "../../../workspace/klipper/klipperbindings.cpp"
#include "../../kwin/kwinbindings.cpp"
#define KICKER_ALL_BINDINGS
#include "../../../workspace/kicker/kicker/core/kickerbindings.cpp"
#include "../../../workspace/kicker/taskbar/taskbarbindings.cpp"
#include "../../../workspace/kdesktop/kdesktopbindings.cpp"
#include "../kxkb/kxkbbindings.cpp"

  // Write all the global keys to kdeglobals.
  // This is needed to be able to check for conflicts with global keys in app's keyconfig
  // dialogs, kdeglobals is empty as long as you don't apply any change in controlcenter/keys.
  // However, avoid writing at every KDE startup, just update them after every rebuild of this file.
        KConfigGroup group( KGlobal::config(), "Global Shortcuts" );
        if( group.readEntry( "Defaults timestamp" ) != __DATE__ __TIME__ ) {
	    kDebug(125) << "KeyModule::init() - Read Config Bindings\n";
	    // Check for old group,
	    if( KGlobal::config()->hasGroup( "Global Keys" ) ) {
		KGlobalAccel::self()->readSettings();
		KGlobal::config()->deleteGroup( "Global Keys", KConfigBase::Global);
	    }
            KGlobal::config()->deleteGroup( "Global Shortcuts", KConfigBase::Global);

	    kDebug(125) << "KeyModule::init() - Write Config Bindings\n";
            KGlobalAccel::self()->writeSettings();
            group.writeEntry( "Defaults timestamp", __DATE__ __TIME__, KConfigBase::Normal|KConfigBase::Global);
        }
	delete actionCollection;
  }
}

#include "main.moc"
