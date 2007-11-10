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
#include <QDesktopWidget>
#include <QProcess>

#include <kglobal.h>
#include <kglobalaccel.h>
#include <klocale.h>
#include <kwindowsystem.h>
#include <kstandarddirs.h>
#include <kaction.h>
#include <kdebug.h>
#include <kconfig.h>
#include <ktoolinvocation.h>
#include <kglobalsettings.h>
#include <kactioncollection.h>
#include <kapplication.h>

#include "x11helper.h"
#include "extension.h"
#include "rules.h"
#ifdef HAVE_XKLAVIER
#include "xklavier_adaptor.h"
#endif
#include "kxkbconfig.h"
#include "layoutmap.h"
#include "kxkbwidget.h"

#include "kxkbcore.h"

#include "kxkbcore.moc"


/*
    This is dummy class just to handle x11 events
*/
class DummyWidget: public QWidget
{
  KxkbCore* kxkb;
public:
    DummyWidget(KxkbCore* kxkb_):
	  kxkb(kxkb_)
	{ }
protected:
    bool x11Event( XEvent * e) { return kxkb->x11EventFilter(e); }
};


KxkbCore::KxkbCore(int mode):
    m_mode(mode),
    m_currentLayout(0),
    m_layoutOwnerMap(NULL),
    m_rules(NULL),
    m_kxkbWidget(NULL),
    actionCollection(NULL)
{
    m_status = 0;

    m_extension = new XKBExtension();
    if( !m_extension->init() ) {
        kError() << "XKB initialization failed, exiting..." << endl;
	m_status = -2;
        return;
    }

    m_layoutOwnerMap = new LayoutMap(m_kxkbConfig);
}


void KxkbCore::setWidget(KxkbWidget* kxkbWidget)
{
    if( m_status < 0 ) {
        kError() << "kxkb did not initialize - ignoring set widget" << endl;
        return;
    }
    
    if( m_kxkbWidget != NULL ) {
        kDebug() << "destroying old kxkb widget";
 	disconnect(m_kxkbWidget, SIGNAL(menuTriggered(QAction*)), this, SLOT(iconMenuTriggered(QAction*)));
	disconnect(m_kxkbWidget, SIGNAL(iconToggled()), this, SLOT(toggled()));
        delete m_kxkbWidget;
    }

    m_kxkbWidget = kxkbWidget;
    if( m_kxkbWidget != NULL ) {
 	connect(m_kxkbWidget, SIGNAL(menuTriggered(QAction*)), this, SLOT(iconMenuTriggered(QAction*)));
	connect(m_kxkbWidget, SIGNAL(iconToggled()), this, SLOT(toggled()));

        if( m_rules != NULL )   // settings already read
            initTray();
    }
}

void KxkbCore::initReactions()
{
    if( m_mode == KXKB_MAIN && actionCollection == NULL ) {
        KApplication::kApplication()->installX11EventFilter(new DummyWidget(this));
    
#ifdef HAVE_XKLAVIER
        XKlavierAdaptor::getInstance(QX11Info::display())->startListening();
#endif
        initKeys();
    }
    if( actionCollection != NULL ) {
        actionCollection->readSettings();
        kDebug() << "kde shortcut" << static_cast<KAction*>(actionCollection->action(0))->globalShortcut().toString();
    }
}

void KxkbCore::initKeys()
{
    actionCollection = new KActionCollection( this );
//    actionCollection->setConfigGlobal(true);
    KAction* a = NULL;
#include "kxkbbindings.cpp"
    connect(a, SIGNAL(triggered()), this, SLOT(toggled()));

    connect(KGlobalSettings::self(), SIGNAL(settingsChanged(int)), SLOT(settingsChanged(int))); 
}

KxkbCore::~KxkbCore()
{
    delete actionCollection;
    delete m_kxkbWidget;
    delete m_rules;
    delete m_extension;
    delete m_layoutOwnerMap;
}

int KxkbCore::newInstance()
{
    if( m_status == 0 && settingsRead() ) {
        initReactions();

        initSwitchingPolicy();
        m_layoutOwnerMap->reset();

        initTray();
        layoutApply();
        return 0;
    }

    return -1;
}

void KxkbCore::settingsChanged(int category)
{
    if ( category != KGlobalSettings::SETTINGS_SHORTCUTS)
        return;

    kDebug() << "global settings changed";
    actionCollection->readSettings();
    if( actionCollection != NULL )
        kDebug() << "kde shortcut" << static_cast<KAction*>(actionCollection->action(0))->globalShortcut().toString();
	//TODO:
	//keys->updateConnections();
}

bool KxkbCore::settingsRead()
{
    m_kxkbConfig.load( KxkbConfig::LOAD_ACTIVE_OPTIONS );

    if( m_mode == KXKB_MAIN ) {
	if( m_kxkbConfig.m_enableXkbOptions ) {
	    if( !m_extension->setXkbOptions(m_kxkbConfig.m_options, m_kxkbConfig.m_resetOldOptions) ) {
        	kDebug() << "Setting XKB options failed!";
	    }
	}
    }

    if ( m_kxkbConfig.m_useKxkb == false ) {
	kWarning() << "Kxkb is disabled, exiting...";
	m_status = -1;
        return false;
    }

    if( m_rules == NULL )
	m_rules = new XkbRules(false);

    if( m_mode == KXKB_MAIN && ! m_kxkbConfig.m_indicatorOnly ) {
	m_currentLayout = m_kxkbConfig.getDefaultLayout();
	initLayoutGroups();
    }
    else {
	updateGroupsFromServer();
    }
	
    if( m_kxkbConfig.m_layouts.count() == 1 ) {
	if( m_kxkbConfig.m_showSingle == false ) {
	    kWarning() << "Kxkb is hidden for single layout";
//	    m_status = -1;
//	    return false;
	}
    }

//	KGlobal::config()->reparseConfiguration(); // kcontrol modified kdeglobals
	//TODO:
//	keys->readSettings();
	//keys->updateConnections();

    return true;
}

void KxkbCore::initSwitchingPolicy()
{
	if( m_kxkbConfig.m_switchingPolicy == SWITCH_POLICY_GLOBAL ) {
		disconnect(KWindowSystem::self(), SIGNAL(activeWindowChanged(WId)), this, SLOT(windowChanged(WId)));
		disconnect(KWindowSystem::self(), SIGNAL(currentDesktopChanged(int)), this, SLOT(windowChanged(WId)));
	}
	else {
		QDesktopWidget desktopWidget;
		if( desktopWidget.numScreens() > 1 && desktopWidget.isVirtualDesktop() == false ) {
			kWarning() << "With non-virtual desktop only global switching policy supported on non-primary screens" ;
			//TODO: find out how to handle that
		}
		
		if( m_kxkbConfig.m_switchingPolicy == SWITCH_POLICY_DESKTOP ) {
		    disconnect(KWindowSystem::self(), SIGNAL(currentDesktopChanged(int)), this, SLOT(desktopChanged(int)));
		    connect(KWindowSystem::self(), SIGNAL(currentDesktopChanged(int)), this, SLOT(desktopChanged(int)));
		}
		else {
		    disconnect(KWindowSystem::self(), SIGNAL(activeWindowChanged(WId)), this, SLOT(windowChanged(WId)));
		    connect(KWindowSystem::self(), SIGNAL(activeWindowChanged(WId)), this, SLOT(windowChanged(WId)));
		}
	}
}

void KxkbCore::initLayoutGroups()
{
    QStringList layouts;
    QStringList variants;
    for(int ii=0; ii<(int)m_kxkbConfig.m_layouts.count(); ii++) {
        LayoutUnit& layoutUnit = m_kxkbConfig.m_layouts[ii];
        layouts << layoutUnit.layout;
        variants << layoutUnit.variant;
    }
    m_extension->setLayoutGroups(m_kxkbConfig.m_model, layouts, variants);
}

void KxkbCore::initTray()
{
    if( m_kxkbWidget != NULL ) {
        bool visible = m_kxkbConfig.m_layouts.count() > 1 || m_kxkbConfig.m_showSingle;
        kDebug() << "initing tray, visible:" << visible;

        m_kxkbWidget->setVisible( visible );
        m_kxkbWidget->setShowFlag(m_kxkbConfig.m_showFlag);
        m_kxkbWidget->initLayoutList(m_kxkbConfig.m_layouts, *m_rules);
        m_kxkbWidget->setCurrentLayout(m_kxkbConfig.m_layouts[m_currentLayout]);
    }
}

// This function activates the keyboard layout specified by the
// configuration members (m_currentLayout)
void KxkbCore::layoutApply()
{
    setLayout(m_currentLayout);
}

// DBUS
bool KxkbCore::setLayout(const QString& layoutPair)
{
    const LayoutUnit layoutUnitKey(layoutPair);
    if( m_kxkbConfig.m_layouts.contains(layoutUnitKey) ) {
        int ind = m_kxkbConfig.m_layouts.indexOf(layoutUnitKey);
        return setLayout( ind );
    }
    return false;
}


// Activates the keyboard layout specified by layout
bool KxkbCore::setLayout(int layout)
{
    bool res = m_extension->setGroup(layout);

    updateIndicator(layout, res);

    return res;
}

void KxkbCore::updateIndicator(int layout, int res)
{
    if( res ) {
  	m_currentLayout = layout;
 	m_layoutOwnerMap->ownerChanged();
	m_layoutOwnerMap->setCurrentLayout(layout);
    }

    if( m_kxkbWidget ) {
//        QString label = m_kxkbConfig.m_layouts[layout].displayName;
//        if( label.isEmpty() )
//            label = .layout.left(3);

        const LayoutUnit& lu = m_kxkbConfig.m_layouts[layout];

	if( res )
	    m_kxkbWidget->setCurrentLayout(lu);
	else
	    m_kxkbWidget->setError(lu.toPair());
    }
}

void KxkbCore::toggled()
{
    if( m_kxkbConfig.m_layouts.count() <= 1 )
        return;

    int layout = m_layoutOwnerMap->getNextLayout();
    setLayout(layout);
}

void KxkbCore::iconMenuTriggered(QAction* action)
{
    int id = action->data().toInt();

    if( KxkbWidget::START_MENU_ID <= id
        && id < KxkbWidget::START_MENU_ID + (int)m_kxkbConfig.m_layouts.count() )
    {
        if( m_kxkbConfig.m_layouts.count() <= 1 )
            return;
            
        int layout = id - KxkbWidget::START_MENU_ID;
        m_layoutOwnerMap->setCurrentLayout( layout );
        setLayout( layout );
    }
    else if (id == KxkbWidget::CONFIG_MENU_ID)
    {
        QStringList lst;
        lst << "keyboard_layout";
	QProcess::startDetached("kcmshell4",lst);
    }
    else if (id == KxkbWidget::HELP_MENU_ID)
    {
        KToolInvocation::invokeHelp(0, "kxkb");
    }
    else
    {
//        emit quit();
    }
}

void KxkbCore::desktopChanged(int desktop)
{
    kDebug() << "desktop changed" << desktop;
    windowChanged(-1);
}

// TODO: we also have to handle deleted windows
void KxkbCore::windowChanged(WId winId)
{
	if( m_kxkbConfig.m_switchingPolicy == SWITCH_POLICY_GLOBAL ) { // should not happen actually
		kDebug() << "windowChanged() signal in GLOBAL switching policy";
		return;
	}

 	kDebug() << "active window changed new WinId: " << winId;

	if( m_kxkbConfig.m_switchingPolicy == SWITCH_POLICY_GLOBAL
		    || winId != X11Helper::UNKNOWN_WINDOW_ID ) {

		m_layoutOwnerMap->ownerChanged();
		int layoutState = m_layoutOwnerMap->getCurrentLayout();

		if( layoutState != m_currentLayout ) {
			setLayout(layoutState);
		}
	}
}



bool KxkbCore::x11EventFilter ( XEvent * event )
{
#ifdef HAVE_XKLAVIER
    XKlavierAdaptor::getInstance(QX11Info::display())->filterEvents(event);
#endif

  if( m_extension->isXkbEvent(event) ) {
//    qApp->x11ProcessEvent ( event );

    if( XKBExtension::isGroupSwitchEvent(event) ) {
      // group changed
  	  int group = m_extension->getGroup();
	  if( group != m_currentLayout ) {
	    kDebug() << "got event: group chagned to " << group;
	    if( group < m_kxkbConfig.m_layouts.count() ) {
		updateIndicator(group, 1);
	    }
	    else {
		kWarning() << "new group is out of my layouts list range";
	    }
	  }
    }
    else if( XKBExtension::isLayoutSwitchEvent(event) ) {
	  kDebug() << "got event: layouts chagned";
	  updateGroupsFromServer();
    }
    else {
//	kDebug() << "other xkb event: ";// + ((XkbEvent*)event)->any.xkb_type;
    }
  }
  return false;
}

int
KxkbCore::updateGroupsFromServer()
{
    kDebug() << "updating groups from server";

#ifdef HAVE_XKLAVIER
	QList<LayoutUnit> lus = XKlavierAdaptor::getInstance(QX11Info::display())->getGroupNames();
	if( lus.count() > 0 ) {
	    if( lus != m_kxkbConfig.m_layouts ) {
		m_kxkbConfig.setConfiguredLayouts(lus);
		m_layoutOwnerMap->reset();
		initTray();
	    }
	    else {
		kDebug() << "no change in layouts - ignoring event";
	    }

  	    int group = m_extension->getGroup();
            if( m_currentLayout != group ) {
	        kDebug() << "got group from server:" << group;
	        updateIndicator(group, 1);
            }
	}
#else
        int group = m_extension->getGroup();
        if( m_currentLayout != group && group < m_kxkbConfig.m_layouts.count() ) {
	    kDebug() << "got group from server:" << group;
	    updateIndicator(group, 1);
        }

	kDebug() << "updating layouts from server is not implemented w/out libxklavier";
#endif
	
    return 0;
}
