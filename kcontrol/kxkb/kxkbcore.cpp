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
#include <ktemporaryfile.h>
#include <kstandarddirs.h>
#include <kaction.h>
#include <kmenu.h>
#include <kdebug.h>
#include <kconfig.h>
#include <ktoolinvocation.h>
#include <kglobalsettings.h>
#include <kactioncollection.h>

//#include "kxkb_adaptor.h"

#include "x11helper.h"
#include "extension.h"
#include "rules.h"
#include "kxkbconfig.h"
#include "layoutmap.h"
#include "kxkbwidget.h"

#include "kxkbcore.h"

#include "kxkbcore.moc"


KxkbCore::KxkbCore(KxkbWidget* kxkbWidget) :
//     : m_prevWinId(X11Helper::UNKNOWN_WINDOW_ID),
      m_rules(NULL),
      m_kxkbWidget(kxkbWidget),
      kWinModule(NULL)
{
    m_extension = new XKBExtension();
    if( !m_extension->init() ) {
        kDebug() << "xkb initialization failed, exiting..." << endl;
        ::exit(1);
    }

    // keep in sync with kcmlayout.cpp
//    keys = new KActionCollection(this);
//    KActionCollection* actionCollection = keys;
//    QAction* a = 0L;
    //TODO:

//#include "kxkbbindings.cpp"
//    KGlobalAccel::self()->readSettings();

    //keys->updateConnections();

    m_layoutOwnerMap = new LayoutMap(kxkbConfig);

//    connect( KGlobalSettings::self(), SIGNAL(settingsChanged(int)),
//             this, SLOT(slotSettingsChanged(int)) );

    //TODO: don't do this if kxkb does not become a daemon
//     new KXKBAdaptor( this );
//	if( !m_kxkbWidget )
//	{
//		m_kxkbWidget = new KxkbSysTrayIcon();
 		connect(m_kxkbWidget, SIGNAL(menuTriggered(QAction*)), this, SLOT(iconMenuTriggered(QAction*)));
		connect(m_kxkbWidget, SIGNAL(iconToggled()), this, SLOT(iconToggled()));
//	}
}


KxkbCore::~KxkbCore()
{
//    deletePrecompiledLayouts();

    delete keys;
    delete m_kxkbWidget;
    delete m_rules;
    delete m_extension;
	delete m_layoutOwnerMap;
	delete kWinModule;
}

int KxkbCore::newInstance()
{
	m_extension->reset();

    if( settingsRead() )
		layoutApply();

    return 0;
}

bool KxkbCore::settingsRead()
{
	kxkbConfig.load( KxkbConfig::LOAD_ACTIVE_OPTIONS );

    if( kxkbConfig.m_enableXkbOptions ) {
		kDebug() << "Setting XKB options " << kxkbConfig.m_options << endl;
		if( !m_extension->setXkbOptions(kxkbConfig.m_options, kxkbConfig.m_resetOldOptions) ) {
            kDebug() << "Setting XKB options failed!" << endl;
        }
    }

    if ( kxkbConfig.m_useKxkb == false ) {
        emit quit(); //qApp->quit();
        return false;
    }

// 	m_prevWinId = X11Helper::UNKNOWN_WINDOW_ID;

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
/*		int m_prevWinId = kWinModule->activeWindow();
		kDebug() << "Active window " << m_prevWinId << endl;*/
	}

	m_layoutOwnerMap->reset();
	m_layoutOwnerMap->setCurrentWindow( X11Helper::UNKNOWN_WINDOW_ID ); //TODO

	if( m_rules == NULL )
		m_rules = new XkbRules(false);

//	for(int ii=0; ii<(int)kxkbConfig.m_layouts.count(); ii++) {
//		LayoutUnit& layoutUnit = kxkbConfig.m_layouts[ii];
//		layoutUnit.defaultGroup = m_rules->getDefaultGroup(layoutUnit.layout, layoutUnit.includeGroup);
//		kDebug() << "default group for " << layoutUnit.toPair() << " is " << layoutUnit.defaultGroup << endl;
//	}

	m_currentLayout = kxkbConfig.getDefaultLayout();

	if( kxkbConfig.m_layouts.count() == 1 ) {
		const LayoutUnit& currentLayout = kxkbConfig.m_layouts[0];
		QString layoutName = currentLayout.layout;
		QString variantName = currentLayout.variant;
// 		QString includeName = m_currentLayout.includeGroup;
// 		int group = m_currentLayout.defaultGroup;

/*		if( !m_extension->setLayout(kxkbConfig.m_model, layoutName, variantName, includeName, false)
				   || !m_extension->setGroup( group ) ) {*/
		if( ! m_extension->setLayout(layoutName, variantName) ) {
			kDebug() << "Error switching to single layout " << currentLayout.toPair() << endl;
			// TODO: alert user
		}

		if( kxkbConfig.m_showSingle == false ) {
			emit quit(); //qApp->quit();
			return false;
		}
 	}
	else {
		QString layouts;
		QString variants;
		for(int ii=0; ii<(int)kxkbConfig.m_layouts.count(); ii++) {
			LayoutUnit& layoutUnit = kxkbConfig.m_layouts[ii];
			layouts += layoutUnit.layout;
			variants += layoutUnit.variant;
			if( ii < kxkbConfig.m_layouts.count() ) {
				layouts += ",";
				variants += ",";
			}
		}
		kDebug() << "initing " << "-layout " << layouts << " - variants " << variants << endl;
//		initPrecompiledLayouts();
		m_extension->setLayout(layouts, variants);
	}

	initTray();

//	KGlobal::config()->reparseConfiguration(); // kcontrol modified kdeglobals
	//TODO:
//	keys->readSettings();
	//keys->updateConnections();

	return true;
}

void KxkbCore::initTray()
{
    kDebug() << "initing tray" << endl;

	m_kxkbWidget->setShowFlag(kxkbConfig.m_showFlag);
	m_kxkbWidget->initLayoutList(kxkbConfig.m_layouts, *m_rules);
	m_kxkbWidget->setCurrentLayout(kxkbConfig.m_layouts[m_currentLayout]);
    kDebug() << "inited tray" << endl;
}

// This function activates the keyboard layout specified by the
// configuration members (m_currentLayout)
void KxkbCore::layoutApply()
{
    setLayout(m_currentLayout);
}

// // kdcop
// bool KxkbCore::setLayout(int layout)
// {
// 	const LayoutUnit layoutUnitKey(layoutPair);
// 	if( kxkbConfig.m_layouts.contains(layoutUnitKey) ) {
// 		int ind = kxkbConfig.m_layouts.indexOf(layoutUnitKey);
// 		return setLayout( kxkbConfig.m_layouts[ind] );
// 	}
// 	return false;
// }


// Activates the keyboard layout specified by 'layoutUnit'
bool KxkbCore::setLayout(int layout)
{
	bool res = false;

// 	res = m_extension->setLayout(kxkbConfig.m_model,
//  					layoutUnit.layout, layoutUnit.variant,
//  					layoutUnit.includeGroup);
	int group = layout;

	res = m_extension->setGroup(group); // not checking for ret - not important

    if( res ) {
        m_currentLayout = layout;
		int winId = kWinModule->activeWindow();
 		m_layoutOwnerMap->setCurrentWindow(winId);
		m_layoutOwnerMap->setCurrentLayout(group);
	}

    if( m_kxkbWidget ) {
		const QString& layoutName = kxkbConfig.m_layouts[layout].toPair(); //displayName;
		if( res )
			m_kxkbWidget->setCurrentLayout(layoutName);
		else
			m_kxkbWidget->setError(layoutName);
	}

    return res;
}

void KxkbCore::iconToggled()
{
//    if ( reason != QSystemTrayIcon::Trigger )
//        return;
//     const LayoutUnit& layout = m_layoutOwnerMap->getNextLayout().layoutUnit;
    int layout = m_layoutOwnerMap->getNextLayout();
    setLayout(layout);
}

void KxkbCore::iconMenuTriggered(QAction* action)
{
	int id = action->data().toInt();

    if( KxkbWidget::START_MENU_ID <= id
        && id < KxkbWidget::START_MENU_ID + (int)kxkbConfig.m_layouts.count() )
    {
//         const LayoutUnit& layout = kxkbConfig.m_layouts[id - KxkbWidget::START_MENU_ID];
        int layout = id - KxkbWidget::START_MENU_ID;
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
        emit quit();
    }
}

// TODO: we also have to handle deleted windows
void KxkbCore::windowChanged(WId winId)
{
	if( kxkbConfig.m_switchingPolicy == SWITCH_POLICY_GLOBAL ) { // should not happen actually
		kDebug() << "windowChanged() signal in GLOBAL switching policy" << endl;
		return;
	}

// 	int group = m_extension->getGroup();

// 	kDebug() << "old WinId: " << m_prevWinId << ", new WinId: " << winId << endl;

//  	if( m_prevWinId != X11Helper::UNKNOWN_WINDOW_ID ) {	// saving layout/group from previous window
//  		kDebug() << "storing " << m_currentLayout.toPair() << ":" << group << " for " << m_prevWinId << endl;
//  		m_layoutOwnerMap->setCurrentWindow(m_prevWinId);
// // 		m_layoutOwnerMap->setCurrentLayout(m_currentLayout);
// 		m_layoutOwnerMap->setCurrentGroup(group);
//  	}

// 	m_prevWinId = winId;

	if( winId != X11Helper::UNKNOWN_WINDOW_ID ) {
		m_layoutOwnerMap->setCurrentWindow(winId);
		int layoutState = m_layoutOwnerMap->getCurrentLayout();

		if( layoutState != m_currentLayout ) {
			setLayout(layoutState);
		}
	}
}


void KxkbCore::slotSettingsChanged(int category)
{
    if ( category != KGlobalSettings::SETTINGS_SHORTCUTS)
		return;

    KGlobal::config()->reparseConfiguration(); // kcontrol modified kdeglobals
    keys->readSettings();
	//TODO:
	//keys->updateConnections();
}
