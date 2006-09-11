/*
    Copyright (C) 2001, S.R.Haque <srhaque@iee.org>.
	Copyright (C) 2006, Andriy Rysin <rysin@kde.org>. Derived from an
    original by Matthias H�zer-Klpfel released under the QPL.
    This file is part of the KDE project

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

DESCRIPTION

    KDE Keyboard Tool. Manages XKB keyboard mappings.
*/
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>

#include <QRegExp>
#include <QStringList>
#include <QImage>
#include <QDesktopWidget>

#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kglobal.h>
#include <kglobalaccel.h>
#include <klocale.h>
#include <kprocess.h>
#include <kwinmodule.h>
#include <kwin.h>
#include <ktempfile.h>
#include <kstandarddirs.h>
#include <kaction.h>
#include <kmenu.h>
#include <kdebug.h>
#include <kconfig.h>
#include <ktoolinvocation.h>
#include <kglobalsettings.h>
#include <kactioncollection.h>

#include "kxkb_adaptor.h"

#include "x11helper.h"
#include "kxkb.h"
#include "extension.h"
#include "rules.h"
#include "kxkbconfig.h"
#include "layoutmap.h"
#include "kxkbwidget.h"

#include "kxkb.moc"


KXKBApp::KXKBApp(bool allowStyles, bool GUIenabled)
    : KUniqueApplication(allowStyles, GUIenabled),
      m_prevWinId(X11Helper::UNKNOWN_WINDOW_ID),
      m_rules(NULL),
      m_kxkbWidget(NULL),
      kWinModule(NULL),
      m_forceSetXKBMap( false )
{
    m_extension = new XKBExtension();
    if( !m_extension->init() ) {
        kDebug() << "xkb initialization failed, exiting..." << endl;
        ::exit(1);
    }

    // keep in sync with kcmlayout.cpp
    keys = new KActionCollection(this);
    KActionCollection* actionCollection = keys;
    KAction* a = 0L;
    //TODO:

#include "kxkbbindings.cpp"
    KGlobalAccel::self()->readSettings();

    //keys->updateConnections();

    m_layoutOwnerMap = new LayoutMap(kxkbConfig);

    connect( KGlobalSettings::self(), SIGNAL(settingsChanged(int)),
             this, SLOT(slotSettingsChanged(int)) );

    //TODO: don't do this if kxkb does not become a daemon
    new KXKBAdaptor( this );
}


KXKBApp::~KXKBApp()
{
//    deletePrecompiledLayouts();

    delete keys;
    delete m_kxkbWidget;
    delete m_rules;
    delete m_extension;
	delete m_layoutOwnerMap;
	delete kWinModule;
}

int KXKBApp::newInstance()
{
	m_extension->reset();

    if( settingsRead() )
		layoutApply();

    return 0;
}

bool KXKBApp::settingsRead()
{
	kxkbConfig.load( KxkbConfig::LOAD_ACTIVE_OPTIONS );

    if( kxkbConfig.m_enableXkbOptions ) {
		kDebug() << "Setting XKB options " << kxkbConfig.m_options << endl;
		if( !m_extension->setXkbOptions(kxkbConfig.m_options, kxkbConfig.m_resetOldOptions) ) {
            kDebug() << "Setting XKB options failed!" << endl;
        }
    }

    if ( kxkbConfig.m_useKxkb == false ) {
        qApp->quit();
        return false;
    }

	m_prevWinId = X11Helper::UNKNOWN_WINDOW_ID;

	if( kxkbConfig.m_switchingPolicy == SWITCH_POLICY_GLOBAL ) {
		delete kWinModule;
		kWinModule = NULL;
	}
	else {
		QDesktopWidget desktopWidget;
		if( desktopWidget.numScreens() > 1 && desktopWidget.isVirtualDesktop() == false ) {
			kWarning() << "With non-virtual desktop only global switching policy supported on non-primary screens" << endl;
			//TODO: find out how to handle that
		}

		if( kWinModule == NULL ) {
			kWinModule = new KWinModule(0, KWinModule::INFO_DESKTOP);
			connect(kWinModule, SIGNAL(activeWindowChanged(WId)), SLOT(windowChanged(WId)));
		}
		m_prevWinId = kWinModule->activeWindow();
		kDebug() << "Active window " << m_prevWinId << endl;
	}

	m_layoutOwnerMap->reset();
	m_layoutOwnerMap->setCurrentWindow( m_prevWinId );

	if( m_rules == NULL )
		m_rules = new XkbRules(false);

	for(int ii=0; ii<(int)kxkbConfig.m_layouts.count(); ii++) {
		LayoutUnit& layoutUnit = kxkbConfig.m_layouts[ii];
		layoutUnit.defaultGroup = m_rules->getDefaultGroup(layoutUnit.layout, layoutUnit.includeGroup);
		kDebug() << "default group for " << layoutUnit.toPair() << " is " << layoutUnit.defaultGroup << endl;
	}

	m_currentLayout = kxkbConfig.getDefaultLayout();

	if( kxkbConfig.m_layouts.count() == 1 ) {
		QString layoutName = m_currentLayout.layout;
		QString variantName = m_currentLayout.variant;
		QString includeName = m_currentLayout.includeGroup;
		int group = m_currentLayout.defaultGroup;

		if( !m_extension->setLayout(kxkbConfig.m_model, layoutName, variantName, includeName, false)
				   || !m_extension->setGroup( group ) ) {
			kDebug() << "Error switching to single layout " << m_currentLayout.toPair() << endl;
			// TODO: alert user
		}

		if( kxkbConfig.m_showSingle == false ) {
			qApp->quit();
			return false;
		}
 	}
	else {
//		initPrecompiledLayouts();
	}

	initTray();

	KGlobal::config()->reparseConfiguration(); // kcontrol modified kdeglobals
	//TODO:
//	keys->readSettings();
	//keys->updateConnections();

	return true;
}

void KXKBApp::initTray()
{
	if( !m_kxkbWidget )
	{
		m_kxkbWidget = new KxkbSysTrayIcon();
 		connect(m_kxkbWidget, SIGNAL(menuActivated(int)), this, SLOT(iconMenuActivated(int)));
		connect(m_kxkbWidget, SIGNAL(iconToggled()), this, SLOT(iconToggled()));
	}

	m_kxkbWidget->setShowFlag(kxkbConfig.m_showFlag);
	m_kxkbWidget->initLayoutList(kxkbConfig.m_layouts, *m_rules);
	m_kxkbWidget->setCurrentLayout(m_currentLayout);
}

// This function activates the keyboard layout specified by the
// configuration members (m_currentLayout)
void KXKBApp::layoutApply()
{
    setLayout(m_currentLayout);
}

// kdcop
bool KXKBApp::setLayout(const QString& layoutPair)
{
	const LayoutUnit layoutUnitKey(layoutPair);
	if( kxkbConfig.m_layouts.contains(layoutUnitKey) ) {
		int ind = kxkbConfig.m_layouts.indexOf(layoutUnitKey);
		return setLayout( kxkbConfig.m_layouts[ind] );
	}
	return false;
}


// Activates the keyboard layout specified by 'layoutUnit'
bool KXKBApp::setLayout(const LayoutUnit& layoutUnit, int group)
{
	bool res = false;

	if( group == -1 )
		group = layoutUnit.defaultGroup;

	res = m_extension->setLayout(kxkbConfig.m_model,
 					layoutUnit.layout, layoutUnit.variant,
 					layoutUnit.includeGroup);
	if( res )
		m_extension->setGroup(group); // not checking for ret - not important

    if( res )
        m_currentLayout = layoutUnit;

    if( m_kxkbWidget ) {
		if( res )
			m_kxkbWidget->setCurrentLayout(layoutUnit);
		else
			m_kxkbWidget->setError(layoutUnit.toPair());
	}

    return res;
}

void KXKBApp::iconToggled()
{
//    if ( reason != QSystemTrayIcon::Trigger )
//        return;
    const LayoutUnit& layout = m_layoutOwnerMap->getNextLayout().layoutUnit;
    setLayout(layout);
}

void KXKBApp::iconMenuActivated(int id)
{
    if( KxkbWidget::START_MENU_ID <= id
        && id < KxkbWidget::START_MENU_ID + (int)kxkbConfig.m_layouts.count() )
    {
        const LayoutUnit& layout = kxkbConfig.m_layouts[id - KxkbWidget::START_MENU_ID];
        m_layoutOwnerMap->setCurrentLayout( layout );
        setLayout( layout );
    }
    else if (id == KxkbWidget::CONFIG_MENU_ID)
    {
        KProcess p;
        p << "kcmshell" << "keyboard_layout";
        p.start(KProcess::DontCare);
    }
    else if (id == KxkbWidget::HELP_MENU_ID)
    {
        KToolInvocation::invokeHelp(0, "kxkb");
    }
    else
    {
        quit();
    }
}

// TODO: we also have to handle deleted windows
void KXKBApp::windowChanged(WId winId)
{
//	kDebug() << "window switch" << endl;
	if( kxkbConfig.m_switchingPolicy == SWITCH_POLICY_GLOBAL ) { // should not happen actually
		kDebug() << "windowChanged() signal in GLOBAL switching policy" << endl;
		return;
	}

	int group = m_extension->getGroup();

	kDebug() << "old WinId: " << m_prevWinId << ", new WinId: " << winId << endl;

	if( m_prevWinId != X11Helper::UNKNOWN_WINDOW_ID ) {	// saving layout/group from previous window
// 		kDebug() << "storing " << m_currentLayout.toPair() << ":" << group << " for " << m_prevWinId << endl;
// 		m_layoutOwnerMap->setCurrentWindow(m_prevWinId);
		m_layoutOwnerMap->setCurrentLayout(m_currentLayout);
		m_layoutOwnerMap->setCurrentGroup(group);
	}

	m_prevWinId = winId;

	if( winId != X11Helper::UNKNOWN_WINDOW_ID ) {
		m_layoutOwnerMap->setCurrentWindow(winId);
		const LayoutState& layoutState = m_layoutOwnerMap->getCurrentLayout();

		if( layoutState.layoutUnit != m_currentLayout ) {
			kDebug() << "switching to " << layoutState.layoutUnit.toPair() << ":" << group << " for "  << winId << endl;
			setLayout( layoutState.layoutUnit, layoutState.group );
		}
		else if( layoutState.group != group ) {	// we need to change only the group
			m_extension->setGroup(layoutState.group);
		}
	}
}


void KXKBApp::slotSettingsChanged(int category)
{
    if ( category != KGlobalSettings::SETTINGS_SHORTCUTS)
		return;

    KGlobal::config()->reparseConfiguration(); // kcontrol modified kdeglobals
    keys->readSettings();
	//TODO:
	//keys->updateConnections();
}

/*
 Viki (onscreen keyboard) has problems determining some modifiers states
 when kxkb uses precompiled layouts instead of setxkbmap. Probably a bug
 in the xkb functions used for the precompiled layouts *shrug*.
*/
void KXKBApp::forceSetXKBMap( bool set )
{
    if( m_forceSetXKBMap == set )
        return;
    m_forceSetXKBMap = set;
    layoutApply();
}

/*Precompiles the keyboard layouts for faster activation later.
This is done by loading each one of them and then dumping the compiled
map from the X server into our local buffer.*/
// void KXKBApp::initPrecompiledLayouts()
// {
//     QStringList dirs = KGlobal::dirs()->findDirs ( "tmp", "" );
//     QString tempDir = dirs.count() == 0 ? "/tmp/" : dirs[0];
//
// 	QValueList<LayoutUnit>::ConstIterator end = kxkbConfig.m_layouts.end();
//
// 	for (QValueList<LayoutUnit>::ConstIterator it = kxkbConfig.m_layouts.begin(); it != end; ++it)
//     {
// 		LayoutUnit layoutUnit(*it);
// //	const char* baseGr = m_includes[layout];
// //	int group = m_rules->getGroup(layout, baseGr);
// //    	if( m_extension->setLayout(m_model, layout, m_variants[layout], group, baseGr) ) {
// 		QString compiledLayoutFileName = tempDir + layoutUnit.layout + '.' + layoutUnit.variant + ".xkm";
// //    	    if( m_extension->getCompiledLayout(compiledLayoutFileName) )
// 		m_compiledLayoutFileNames[layoutUnit.toPair()] = compiledLayoutFileName;
// //	}
// //	else {
// //    	    kDebug() << "Error precompiling layout " << layout << endl;
// //	}
//     }
// }


const char * DESCRIPTION =
  I18N_NOOP("A utility to switch keyboard maps");

extern "C" KDE_EXPORT int kdemain(int argc, char *argv[])
{
    KAboutData about("kxkb", I18N_NOOP("KDE Keyboard Tool"), "1.0",
                     DESCRIPTION, KAboutData::License_LGPL,
                     "Copyright (C) 2001, S.R.Haque\n(C) 2002-2003, 2006 Andriy Rysin");
    KCmdLineArgs::init(argc, argv, &about);
    KXKBApp::addCmdLineOptions();

    if (!KXKBApp::start())
        return 0;

    KXKBApp app;
    app.disableSessionManagement();
    app.exec();
    return 0;
}
