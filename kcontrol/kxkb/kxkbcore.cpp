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

#include <QDesktopWidget>
#include <QProcess>

#include <kaboutdata.h>
#include <kcmdlineargs.h>
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

//#include "kxkb_component.h"

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


KxkbCore::KxkbCore(QWidget* parent, int mode, int controlType, int widgetType):
    m_mode(mode),
    m_controlType(controlType),
    m_widgetType(widgetType),
    m_layoutOwnerMap(NULL),
    m_rules(NULL),
    m_parentWidget(parent),
    m_kxkbWidget(NULL)
{
    m_status = 0;

    m_extension = new XKBExtension();
    if( !m_extension->init() ) {
        kError() << "xkb initialization failed, exiting...";
	m_status = -2;
//        emit quit();
        return;
    }

    m_layoutOwnerMap = new LayoutMap(m_kxkbConfig);
}


void KxkbCore::initWidget()
{
    if( ! m_kxkbWidget )
    {
        if( m_widgetType == KxkbWidget::WIDGET_TRAY )
            m_kxkbWidget = new KxkbSysTrayIcon();
        else
            m_kxkbWidget = new KxkbLabel(m_controlType, m_parentWidget);
            
 	connect(m_kxkbWidget, SIGNAL(menuTriggered(QAction*)), this, SLOT(iconMenuTriggered(QAction*)));
	connect(m_kxkbWidget, SIGNAL(iconToggled()), this, SLOT(toggled()));

        if( m_mode == KXKB_MAIN ) {
	    KApplication::kApplication()->installX11EventFilter(new DummyWidget(this));
    
#ifdef HAVE_XKLAVIER
	    XKlavierAdaptor::getInstance(QX11Info::display())->startListening();
#endif
    
	    initKeys();
        }
    }
}

void KxkbCore::initKeys()
{
    m_actions = new KActionCollection( this );
        QAction* a = m_actions->addAction( I18N_NOOP("Switch keyboard layout") );
        a->setText( i18n( I18N_NOOP( "Switch keyboard layout" ) ) );
        qobject_cast<KAction*>( a )->setGlobalShortcut(KShortcut(Qt::ALT+Qt::CTRL+Qt::Key_K));
        connect( a, SIGNAL(triggered(bool)), this, SLOT(toggled()) );

  // TODO: keyboard bindings
    //globalKeys = KGlobalAccel::self();
//    m_keys* = new KActionCollection(this);

//	#include "kxkbbindings.cpp"
    //keys->readSettings();
    //keys->updateConnections();

//    connect( KGlobalSettings::self(), SIGNAL(settingsChanged(int)),
//             this, SLOT(slotSettingsChanged(int)) );
}

KxkbCore::~KxkbCore()
{
    delete m_actions;
    delete m_kxkbWidget;
    delete m_rules;
    delete m_extension;
    delete m_layoutOwnerMap;
}

int KxkbCore::newInstance()
{
    if( m_status == 0 && settingsRead() ) {
        initWidget();

        initSwitchingPolicy();

        m_layoutOwnerMap->reset();
        initTray();

        layoutApply();
        return 0;
    }

    return -1;
}

void KxkbCore::slotSettingsChanged(int category)
{
    if ( category != KGlobalSettings::SETTINGS_SHORTCUTS)
		return;

#ifdef __GNUC__
#warning TODO PORT ME (KGlobalAccel related)
#endif

    KGlobal::config()->reparseConfiguration(); // kcontrol modified kdeglobals
//    m_keys->readSettings();
	//TODO:
	//keys->updateConnections();
}

bool KxkbCore::settingsRead()
{
    m_kxkbConfig.load( KxkbConfig::LOAD_ACTIVE_OPTIONS );

    if( m_mode == KXKB_MAIN ) {
	if( m_kxkbConfig.m_enableXkbOptions ) {
            QString options = m_kxkbConfig.m_options.join(",");
	    if( !m_extension->setXkbOptions(options, m_kxkbConfig.m_resetOldOptions) ) {
        	kDebug() << "Setting XKB options failed!";
	    }
	}
    }

    if ( m_kxkbConfig.m_useKxkb == false ) {
	kWarning() << "Kxkb is disabled, exiting...";
	m_status = -1;
//        emit quit();
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
	    kWarning() << "Kxkb is disabled for single layout";
	    m_status = -1;
//	    emit quit();
	    return false;
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
	if( m_kxkbConfig.m_layouts.count() == 1 ) {
		const LayoutUnit& currentLayout = m_kxkbConfig.m_layouts[0];
		QString layoutName = currentLayout.layout;
		QString variantName = currentLayout.variant;

/*		if( !m_extension->setLayout(kxkbConfig.m_model, layoutName, variantName, includeName, false)
				   || !m_extension->setGroup( group ) ) {*/
		if( ! m_extension->setLayoutGroups(layoutName, variantName) ) {
			kDebug() << "Error switching to single layout " << currentLayout.toPair();
			// TODO: alert user
		}

 	}
	else {
		QString layouts;
		QString variants;
		for(int ii=0; ii<(int)m_kxkbConfig.m_layouts.count(); ii++) {
			LayoutUnit& layoutUnit = m_kxkbConfig.m_layouts[ii];
			layouts += layoutUnit.layout;
			variants += layoutUnit.variant;
			if( ii < m_kxkbConfig.m_layouts.count()-1 ) {
				layouts += ',';
				variants += ',';
			}
		}
		kDebug() << "initing " << "-layout " << layouts << " -variants " << variants;
		m_extension->setLayoutGroups(layouts, variants);
	}
}

void KxkbCore::initTray()
{
    kDebug() << "initing tray";

    m_kxkbWidget->setShowFlag(m_kxkbConfig.m_showFlag);
    m_kxkbWidget->initLayoutList(m_kxkbConfig.m_layouts, *m_rules);
    m_kxkbWidget->setCurrentLayout(m_kxkbConfig.m_layouts[m_currentLayout]);
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


// Activates the keyboard layout specified by 'layoutUnit'
bool KxkbCore::setLayout(int layout)
{
    bool res = m_extension->setGroup(layout); // not checking for ret - not important

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
	const QString& layoutName = m_kxkbConfig.m_layouts[layout].toPair(); //displayName;
	if( res )
	    m_kxkbWidget->setCurrentLayout(layoutName);
	else
	    m_kxkbWidget->setError(layoutName);
    }
}

void KxkbCore::toggled()
{
    int layout = m_layoutOwnerMap->getNextLayout();
    setLayout(layout);
}

void KxkbCore::iconMenuTriggered(QAction* action)
{
    int id = action->data().toInt();

    if( KxkbWidget::START_MENU_ID <= id
        && id < KxkbWidget::START_MENU_ID + (int)m_kxkbConfig.m_layouts.count() )
    {
        int layout = id - KxkbWidget::START_MENU_ID;
        m_layoutOwnerMap->setCurrentLayout( layout );
        setLayout( layout );
    }
    else if (id == KxkbWidget::CONFIG_MENU_ID)
    {
        QStringList lst;
        lst<< "keyboard_layout";
	QProcess::startDetached("kcmshell4",lst);
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
	    kDebug() << "got group from server:" << group;
	    updateIndicator(group, 1);
	}
#else
	kDebug() << "updating layouts from server is not implemented w/out libxklavier";
#endif
	
    return 0;
}
