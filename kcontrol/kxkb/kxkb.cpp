/*
    Copyright (C) 2001, S.R.Haque <srhaque@iee.org>. Derived from an
    original by Matthias Hölzer-Klüpfel released under the QPL.
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
#include <QFile>
#include <QToolTip>
#include <QStringList>
#include <QImage>
//Added by qt3to4:
#include <QPixmap>
#include <QMouseEvent>

#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kglobalaccel.h>
#include <klocale.h>
#include <kprocess.h>
#include <kiconloader.h>
#include <kiconeffect.h>
#include <kwinmodule.h>
#include <kwin.h>
#include <ktempfile.h>
#include <kstandarddirs.h>
#include <kipc.h>
#include <kaction.h>
#include <kmenu.h>
#include <QX11Info>

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#define explicit int_explicit        // avoid compiler name clash in XKBlib.h
#include <X11/XKBlib.h>
#include <ktoolinvocation.h>
#undef explicit

#include "kxkb.h"
#include "extension.h"
#include "rules.h"
#include "kxkb.moc"
#include "pixmap.h"


TrayWindow::TrayWindow(QWidget *parent, const char *name)
    : KSystemTray(parent, name),
    mPrevMenuCount(0)
{
}


void TrayWindow::setCurrentLayout(const QString& layout)
{
    this->setToolTip( mDescriptionMap[layout]);

    KIconEffect iconeffect;

    setPixmap( iconeffect.apply(LayoutIcon::findPixmap(layout, m_showFlag), K3Icon::Panel, K3Icon::DefaultState) );
}

void TrayWindow::setError(const QString& layout)
{
    QString msg = i18n("Error changing keyboard layout to '%1'", layout);
    this->setToolTip( msg);

    setPixmap(LayoutIcon::findPixmap("error", m_showFlag));
}

void TrayWindow::setLayouts(const QStringList& layouts, const KeyRules& rules)
{
    int index = contextMenu()->indexOf(0);
    KMenu* menu = contextMenu();

    mDescriptionMap.clear();
    menu->clear();
    menu->insertTitle( qApp->windowIcon().pixmap(IconSize(K3Icon::Small),IconSize(K3Icon::Small)), KInstance::caption() );
    
    KIconEffect iconeffect;

    int cnt = 0;
    QStringList::ConstIterator it;
    for (it=layouts.begin(); it != layouts.end(); ++it)
    {
        const QPixmap pix = iconeffect.apply(LayoutIcon::findPixmap(*it, m_showFlag), K3Icon::Small, K3Icon::DefaultState);
        contextMenu()->insertItem(QIcon(pix), i18n((rules.layouts()[*it])), cnt++);
        mDescriptionMap.insert(*it, i18n((rules.layouts()[*it])));
    }    

    contextMenu()->insertItem(QIcon(SmallIcon("configure")), i18n("Configure..."), cnt++);
    contextMenu()->insertSeparator();
    contextMenu()->insertItem(QIcon(SmallIcon("help")), i18n("Help"), cnt++);

    if( index != -1 ) { //not first start
	menu->insertSeparator();
	KAction* quitAction = KStdAction::quit(this, SIGNAL(quitSelected()), actionCollection());
        if (quitAction)
    	    quitAction->plug(menu);
    }
}

void TrayWindow::mouseReleaseEvent(QMouseEvent *ev)
{
    if (ev->button() == Qt::LeftButton)
        emit toggled();
    KSystemTray::mouseReleaseEvent(ev);
}



KXKBApp::KXKBApp(bool allowStyles, bool GUIenabled)
    : KUniqueApplication(allowStyles, GUIenabled),
    prevWinId(0),
    m_rules(NULL),
    m_tray(NULL),
    kWinModule(NULL),
    m_forceSetXKBMap( false )
{
    m_extension = new XKBExtension;
    if( !m_extension->init() ) {
	kDebug() << "xkb initialization failed, exiting..." << endl;
	::exit(1);
    }

    // keep in sync with kcmlayout.cpp
    keys = new KGlobalAccel(this);
#include "kxkbbindings.cpp"
    keys->updateConnections();

    m_lastLayout = new QQueue<QString*>;
    //m_lastLayout->setAutoDelete(TRUE);

    connect( this, SIGNAL(settingsChanged(int)), SLOT(slotSettingsChanged(int)) );
    addKipcEventMask( KIPC::SettingsChanged );
}


KXKBApp::~KXKBApp()
{
    deletePrecompiledLayouts();
  	qDeleteAll(*m_lastLayout);
	m_lastLayout->clear();
    delete keys;
    delete m_tray;
    delete m_rules;
    delete m_extension;
    delete kWinModule;
}

int KXKBApp::newInstance()
{
    if( !m_compiledLayoutFileNames.isEmpty() )
	deletePrecompiledLayouts();
	
    if( settingsRead() )
	layoutApply();
	
    return 0;
}

bool KXKBApp::settingsRead()
{
    KConfig *config = new KConfig("kxkbrc", true);
    config->setGroup("Layout");

// Even if the layouts have been disabled we still want to set Xkb options
// user can always switch them off now in the "Options" tab
    bool enableXkbOptions = config->readEntry("EnableXkbOptions", true);
    if( enableXkbOptions ) {
	m_resetOldOptions = config->readEntry("ResetOldOptions", false);
	m_options = config->readEntry("Options", "");
	if( !m_extension->setXkbOptions(m_options, m_resetOldOptions) ) {
	    kDebug() << "Setting XKB options failed!" << endl;
	}
    }
    bool enabled = config->readEntry("Use", false);
    if (!enabled)
    {
        delete config;
	kapp->quit();
        return false;
    }

    QString layoutOwner = config->readEntry("SwitchMode", "Global");

    if( layoutOwner != "WinClass" && layoutOwner != "Window" ) {
	m_layoutOwnerMap.setMode(swpGlobal);
	delete kWinModule;
	kWinModule = 0;
    }
    else {
	if( !kWinModule ) {
	    kWinModule = new KWinModule(0, KWinModule::INFO_DESKTOP);
	    connect(kWinModule, SIGNAL(activeWindowChanged(WId)), SLOT(windowChanged(WId)));
	}
	if( layoutOwner == "WinClass" ) {
	    m_layoutOwnerMap.setMode(swpWinClass);
	}
	else if( layoutOwner == "Window" ) {
	    m_layoutOwnerMap.setMode(swpWindow);
	}
    }

    m_rules = new KeyRules();
    m_model = config->readEntry("Model", "pc104");
    m_layout = config->readEntry("Layout", "");
    m_defaultLayout = m_layout;


    m_list = config->readEntry("Additional", QStringList());
    if (!m_list.contains(m_layout))
    {
        m_list.prepend(m_layout);
    }

    kDebug() << "found " << m_list.count() << " layouts" << endl;

// reading variants
    m_variants.clear();
    QStringList vars = config->readEntry("Variants", QStringList());
    m_rules->parseVariants(vars, m_variants);

    m_includes.clear();
    if( m_rules->isXFree_v43() ) {
	QStringList incs = config->readEntry("Includes", QStringList());
	m_rules->parseVariants(incs, m_includes, false);
    }

    if( m_list.count() >= 2 ) {
	precompileLayouts();
    }
    else {
	int group = m_rules->getGroup(m_layout, m_includes[m_layout]);
        if( !m_extension->setLayout(m_model, m_layout, m_variants[m_layout], group, m_includes[m_layout]) ) {
	    kDebug() << "Error switching to single layout " << m_layout << endl;
// kapp->quit();
	}
	bool showSingle = config->readEntry("ShowSingle", false);
	if( !showSingle ) {
    	    delete config;
	    kapp->quit();
    	    return false;
	}
    }

    m_stickySwitching = config->readEntry("StickySwitching", false);
    m_stickySwitchingDepth = config->readEntry("StickySwitchingDepth", 1);

    if( !m_tray )
    {
	m_tray = new TrayWindow(0, 0);
	connect(m_tray->contextMenu(), SIGNAL(activated(int)), this, SLOT(menuActivated(int)));
	connect(m_tray, SIGNAL(toggled()), this, SLOT(toggled()));
    }

    bool showFlag = config->readEntry("ShowFlag", true);
    m_tray->setShowFlag(showFlag);
    m_tray->setLayouts(m_list, *m_rules);
    m_tray->setCurrentLayout(m_layout);
    m_tray->show();

    delete config;

    KGlobal::config()->reparseConfiguration(); // kcontrol modified kdeglobals
    keys->readSettings();
    keys->updateConnections();
    
    return true;
}

// This function activates the keyboard layout specified by the
// configuration members (m_layout)
void KXKBApp::layoutApply()
{
    setLayout(m_layout);
}

// Activates the keyboard layout specified by 'layout'
bool KXKBApp::setLayout(const QString& layout)
{
    bool res = false;
    const char* baseGr = m_includes[layout]; 
    m_group = m_rules->getGroup(layout, baseGr);

    if ( m_compiledLayoutFileNames.contains(layout) && !m_forceSetXKBMap )
    {
        res = m_extension->setCompiledLayout(m_compiledLayoutFileNames[layout]);
//	kDebug() << "setting compiled for " << layout << ": " << res << endl;
        if( res )
	    m_extension->setGroup(m_group);
    }

    if ( !res ) // try not compiled layout, store compiled if success
    {
        res = m_extension->setLayout(m_model, layout, m_variants[layout], m_group, baseGr);
//	kDebug() << "setting non-compiled for " << layout << ": " << res << endl;
        if( res )
            m_extension->getCompiledLayout(m_compiledLayoutFileNames[layout]);
    }

    if( res )
        m_layout = layout;
    
    if (m_tray) {
	if( res ) {
	    m_tray->setCurrentLayout(layout);
	}
	else  
	    m_tray->setError(layout);
    }
    
    return res;
}

// Precompiles the keyboard layouts for faster activation later.
// This is done by loading each one of them and then dumping the compiled
// map from the X server into our local buffer.
void KXKBApp::precompileLayouts()
{
    QStringList dirs = KGlobal::dirs()->findDirs ( "tmp", "" );
    QString tempDir = dirs.count() == 0 ? "/tmp/" : dirs[0]; 

    QStringList::ConstIterator end = m_list.end();

    for (QStringList::ConstIterator it = m_list.begin(); it != end; ++it)
    {
	QString layout(*it);
//	const char* baseGr = m_includes[layout]; 
//	int group = m_rules->getGroup(layout, baseGr);
//    	if( m_extension->setLayout(m_model, layout, m_variants[layout], group, baseGr) ) {
    	    QString compiledLayoutFileName = tempDir + layout + ".xkm";
//    	    if( m_extension->getCompiledLayout(compiledLayoutFileName) )
    		m_compiledLayoutFileNames[layout] = compiledLayoutFileName;
//	}
//	else {
//    	    kDebug() << "Error precompiling layout " << layout << endl;
//	}
    }
}

// Deletes the precompiled layouts stored in temporary files
void KXKBApp::deletePrecompiledLayouts()
{
    QMapIterator<QString,QString> it(m_compiledLayoutFileNames);
    while (it.hasNext())
    {
       unlink(QFile::encodeName(it.next().data()));
    }
    m_compiledLayoutFileNames.clear();
}

void KXKBApp::toggled()
{
    int index = m_list.findIndex(m_layout);
//    if (++index >= m_list.count())
//        index = 0;
   int original_index = index;

   if (m_stickySwitching)
   {
	// get next layout from queue
	if ((int)m_lastLayout->count() >= m_stickySwitchingDepth)
	{
	    while (m_lastLayout->count() > 0)
	    {
		QString *tmp = m_lastLayout->dequeue();
	        int i = m_list.findIndex(*tmp);
		delete tmp;
	        if (i != -1)
		{
		    index = i;
		    break;
		}
	    }
	}
	m_lastLayout->enqueue(new QString(m_layout));
	
	// shrink queue if m_stickySwitchingDepth has been decremented
	
	while ((int)m_lastLayout->count() > m_stickySwitchingDepth)
	{
	    delete m_lastLayout->dequeue();
	}
   }
   
   if (!m_stickySwitching || index == original_index)
   {
 	if (++index >= m_list.count())
   	    index = 0;
   }
    m_layout = m_list[index];
    layoutApply();
}

// we also have to handle deleted windows

void KXKBApp::windowChanged(WId winId)
{
    if( m_layoutOwnerMap.getMode() == swpGlobal )	// should not happen actually
	return;
    

    int group = m_extension->getGroup();
    
    if( prevWinId ) {	// saving layout/group from previous window
			// this will not work for the window activated before kxkb start :(
	    LayoutInfo layoutInfo(m_layout, group, m_lastLayout);
	    m_layoutOwnerMap.setLayout(prevWinId, layoutInfo);
    }
    
    prevWinId = winId;

    const LayoutInfo& layoutInfo = m_layoutOwnerMap.getLayout(winId);
    
    if( layoutInfo.layout.isEmpty() ) {	// setting default layout/group
	m_layout = m_defaultLayout;
 	m_lastLayout = new QQueue<QString*>();
 	//m_lastLayout->setAutoDelete(TRUE);
	layoutApply();
	return;
    }

    m_lastLayout = layoutInfo.getLastLayout();
	
    if( layoutInfo.layout != m_layout ) {
	m_layout = layoutInfo.layout;
        layoutApply();	// we have to add group parameter to settingApply() ??
	m_extension->setGroup(layoutInfo.group);
    }
    else if( layoutInfo.group != group ) {	// we need to change only the group
	m_extension->setGroup(layoutInfo.group);
    }
    // nothing to do
}

void KXKBApp::menuActivated(int id)
{
    if (0 <= id && id < (int)m_list.count())
    {
	if (m_stickySwitching)
	{
	    if ((int)m_lastLayout->count() >= m_stickySwitchingDepth)
		delete m_lastLayout->dequeue();
	    m_lastLayout->enqueue(new QString(m_layout));
	}

        m_layout = m_list[id];
        layoutApply();
    }
    else if (id == (int)m_list.count())
    {
        KProcess p;
        p << "kcmshell" << "keyboard_layout";
        p.start(KProcess::DontCare);
    }
    else if (id == (int)m_list.count()+1)
    {
	KToolInvocation::invokeHelp(0, "kxkb");
    }
    else
    {
        quit();
    }
}


void KXKBApp::slotSettingsChanged(int category)
{
    if ( category != KApplication::SETTINGS_SHORTCUTS) return;

    KGlobal::config()->reparseConfiguration(); // kcontrol modified kdeglobals
    keys->readSettings();
    keys->updateConnections();
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

static QString windowClass(WId winId)
{
  unsigned long nitems_ret, bytes_after_ret;
  unsigned char* prop_ret;
  Atom     type_ret;
  int      format_ret;
  Window w = (Window)winId;	// suppose WId == Window
  QString  property;

  if((XGetWindowProperty(QX11Info::display(), w, XA_WM_CLASS, 0L, 256L, 0, XA_STRING,
			&type_ret, &format_ret, &nitems_ret,
			&bytes_after_ret, &prop_ret) == Success) && (type_ret != None)) {
    property = QString::fromLocal8Bit(reinterpret_cast<char*>(prop_ret));
    XFree(prop_ret);
  }
  return property;
}

SwitchingPolicy LayoutMap::getMode() 
{
    return m_ownerMode;
}


void LayoutMap::setMode(SwitchingPolicy mode)
{
    m_ownerMode = mode;

    m_appLayouts.clear();
    m_winLayouts.clear();
}


const LayoutInfo& LayoutMap::getLayout(WId winId)
{

    static LayoutInfo emptyInfo;

    // we should not et here with mode==Global
    switch( m_ownerMode ) {

	case swpWinClass:
	{
	    QString winClass = windowClass(winId);
	    WinClassLayoutMap::Iterator it = m_appLayouts.find(winClass);

	    if( it == m_appLayouts.end() ) {
		return emptyInfo; //m_defaultLayout;
	    }
	    else {
		return it.data();
	    }
//kDebug("getLayout: winId %lu, pid %lu, %s", winId, pid, newLayout.toLatin1()); 
	}
	break;

	case swpWindow:
	{
	    WinLayoutMap::Iterator it = m_winLayouts.find(winId);

	    if( it == m_winLayouts.end() ) {
		return emptyInfo; //m_defaultLayout;
	    }
	    else {
		return it.data();
	    }
	}
	break;

	default: assert( false );
    }
    
    return emptyInfo;
}


void LayoutMap::setLayout(WId winId, const LayoutInfo& info)
{
//    LayoutInfo& layoutInfo;
    switch( m_ownerMode ) {

	case swpWinClass:
	{
	    QString winClass = windowClass(winId);
	    m_appLayouts[winClass] = info;
	    break;
	}

	case swpWindow:
	{
	    m_winLayouts[winId] = info;
	    break;
	}

	default:
	{
	    // calling this function while in Global switching mode
	    // doesn't make sense
	    assert( false );
	    break;
	}
    }
}


const char * DESCRIPTION =
  I18N_NOOP("A utility to switch keyboard maps");

extern "C" KDE_EXPORT int kdemain(int argc, char *argv[])
{
    KAboutData about("kxkb", I18N_NOOP("KDE Keyboard Tool"), "0.9",
                     DESCRIPTION, KAboutData::License_LGPL,
                     "Copyright (C) 2001, S.R.Haque\n(C) 2002-2003 Andriy Rysin");
    KCmdLineArgs::init(argc, argv, &about);
    KXKBApp::addCmdLineOptions();

    if (!KXKBApp::start())
        return 0;

    KXKBApp app;
    app.disableSessionManagement();
    app.exec();
    return 0;
}
