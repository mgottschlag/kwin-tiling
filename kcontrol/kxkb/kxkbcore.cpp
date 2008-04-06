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
    m_eventsHandled(false),
    m_error(false),
    m_layoutOwnerMap(NULL),
    m_rules(NULL),
    m_kxkbWidget(NULL),
    actionCollection(NULL),
    m_dummyWidget(NULL)
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

KxkbCore::~KxkbCore()
{
    cleanup();
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


void KxkbCore::initReactions()
{
    if( ! m_eventsHandled ) {
        m_dummyWidget = new DummyWidget(this);
        KApplication::kApplication()->installX11EventFilter(m_dummyWidget);
#ifdef HAVE_XKLAVIER
        XKlavierAdaptor::getInstance(QX11Info::display())->startListening();
#endif
        m_eventsHandled = true;
    }

    initKDEShortcut();
}

void KxkbCore::cleanup()
{
    kDebug() << "cleaning up";
    if( m_dummyWidget != NULL ) {
#ifdef HAVE_XKLAVIER
        XKlavierAdaptor::getInstance(QX11Info::display())->stopListening();
#endif
        KApplication::kApplication()->removeX11EventFilter(m_dummyWidget);
        delete m_dummyWidget;
        m_dummyWidget = NULL;
        m_eventsHandled = false;
    }
    stopKDEShortcut();
}

void KxkbCore::initKDEShortcut()
{
    if( m_mode == KXKB_MAIN && !m_kxkbConfig.m_indicatorOnly ) {        // TODO: should component react to kde shortcut?
        if( actionCollection == NULL ) {
            actionCollection = new KActionCollection( this );
            KAction* a = NULL;
#include "kxkbbindings.cpp"
            connect(a, SIGNAL(triggered()), this, SLOT(toggled()));
            connect(KGlobalSettings::self(), SIGNAL(settingsChanged(int)), this, SLOT(settingsChanged(int)));
        }
        KAction* kAction = static_cast<KAction*>(actionCollection->action(0));
        kDebug() << "kde shortcut" << kAction->globalShortcut().toString();
    }
    else {
        stopKDEShortcut();
    }
}

const KShortcut* KxkbCore::getKDEShortcut() {
    if( actionCollection == NULL )
        return NULL;
    KAction* kAction = static_cast<KAction*>(actionCollection->action(0));
    if (kAction == NULL) 
        return NULL;
        
    return &kAction->globalShortcut();
}

void KxkbCore::stopKDEShortcut()
{
    if( actionCollection != NULL ) {
        KAction* kAction = static_cast<KAction*>(actionCollection->action(0));
        disconnect(kAction, SIGNAL(triggered()), this, SLOT(toggled()));
        disconnect(KGlobalSettings::self(), SIGNAL(settingsChanged(int)), this, SLOT(settingsChanged(int)));
        actionCollection->clear();
        delete actionCollection;
        actionCollection = NULL;
    }
}

void KxkbCore::settingsChanged(int category)
{
    if ( category == KGlobalSettings::SETTINGS_SHORTCUTS) {
        // TODO: can we do it more efficient or recreating action collection is the only way?
        stopKDEShortcut();
        initKDEShortcut();
    }
}

bool KxkbCore::settingsRead()
{
    m_kxkbConfig.load( KxkbConfig::LOAD_ACTIVE_OPTIONS );

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
        // for component or indicator we don't need owner map
        m_kxkbConfig.m_switchingPolicy = SWITCH_POLICY_GLOBAL;
        updateGroupsFromServer();
    }
	
    if( m_kxkbConfig.m_layouts.count() == 1 ) {
	if( m_kxkbConfig.m_showSingle == false ) {
	    kWarning() << "Kxkb is hidden for single layout";
//	    m_status = -1;
//	    return false;
	}
    }

    return true;
}

void KxkbCore::initSwitchingPolicy()
{
    disconnect(KWindowSystem::self(), SIGNAL(activeWindowChanged(WId)), this, SLOT(windowChanged(WId)));
    disconnect(KWindowSystem::self(), SIGNAL(currentDesktopChanged(int)), this, SLOT(desktopChanged(int)));

    if( m_kxkbConfig.m_switchingPolicy != SWITCH_POLICY_GLOBAL 
            && m_mode == KXKB_MAIN && !m_kxkbConfig.m_indicatorOnly ) {
	QDesktopWidget desktopWidget;
	if( desktopWidget.numScreens() > 1 && desktopWidget.isVirtualDesktop() == false ) {
	    kWarning() << "With non-virtual desktop only global switching policy supported on non-primary screens" ;
	    //TODO: find out how to handle that
	}

	if( m_kxkbConfig.m_switchingPolicy == SWITCH_POLICY_DESKTOP ) {
	    connect(KWindowSystem::self(), SIGNAL(currentDesktopChanged(int)), this, SLOT(desktopChanged(int)));
	}
	else {
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
    m_extension->setLayoutGroups(m_kxkbConfig.m_model, layouts, variants, 
                                m_kxkbConfig.m_options, m_kxkbConfig.m_resetOldOptions);
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

void KxkbCore::initTray()
{
    if( m_kxkbWidget != NULL ) {
        bool visible = m_kxkbConfig.m_layouts.count() > 1 || m_kxkbConfig.m_showSingle;
        kDebug() << "initing tray, visible:" << visible;

        m_kxkbWidget->setShowFlag(m_kxkbConfig.m_showFlag);
        m_kxkbWidget->initLayoutList(m_kxkbConfig.m_layouts, *m_rules);
        m_kxkbWidget->setCurrentLayout(m_kxkbConfig.m_layouts[m_currentLayout]);
        m_kxkbWidget->setVisible( visible );
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
    if( layout >= GROUP_LIMIT || layout >= m_kxkbConfig.m_layouts.count() ) {
        m_error = true;
        
        if( m_kxkbWidget ) {
            LayoutUnit lu( i18n("Group %1", layout+1), "" );
            lu.setDisplayName( QString("%1").arg(layout+1) );
            m_kxkbWidget->setCurrentLayout(lu);
        }
        kWarning() << "group is out of my range, seems like old style groups are used";
        return;
    }

    m_error = ( res > 0 );

    if( res ) {
  	m_currentLayout = layout;
 	m_layoutOwnerMap->ownerChanged();
	m_layoutOwnerMap->setCurrentLayout(layout);
    }

    if( m_kxkbWidget ) {
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
	QProcess::startDetached("kcmshell4", lst);
    }
    else if (id == KxkbWidget::HELP_MENU_ID)
    {
        KToolInvocation::invokeHelp(0, "kxkb");
    }
    else
    {
//        quit;
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
	  if( group != m_currentLayout || m_error ) {
	        kDebug() << "got event: group chagned to " << group;
		updateIndicator(group, 1);
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
    XkbConfig xkbConfig = XKlavierAdaptor::getInstance(QX11Info::display())->getGroupNames();
#else
    XkbConfig xkbConfig = X11Helper::getGroupNames(QX11Info::display());
#endif

    int group = m_extension->getGroup();
    kDebug() << " active group" << group;
    
    const QList<LayoutUnit>& lus = xkbConfig.layouts;
    if( lus.count() > 0 ) {
        if( lus != m_kxkbConfig.m_layouts ) {
            m_kxkbConfig.setConfiguredLayouts(xkbConfig);
            m_layoutOwnerMap->reset();
            initTray();
        }
        else {
            kDebug() << "no change in layouts";
        }

        updateIndicator(group, 1);
    }
    else {
        kWarning() << "failed to get layouts from server";
        if( m_currentLayout != group && group < m_kxkbConfig.m_layouts.count() ) {
	    kDebug() << "...tryin to set at least group" << group;
	    updateIndicator(group, 1);
        }
    }
//	kDebug() << "updating layouts from server is not implemented w/out libxklavier";
	
    return 0;
}
