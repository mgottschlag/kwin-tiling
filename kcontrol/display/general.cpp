//
// KDE Display color scheme setup module
//
// Copyright (c)  Mark Donohoe 1997
//

/*
 *
 *    $Id$
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <qgroupbox.h>
#include <qbuttongroup.h>
#include <qlabel.h>
#include <qpixmap.h>
#include <qpushbutton.h>
#include <qfiledialog.h>
#include <qradiobutton.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qlayout.h>
#include <qmessagebox.h>

#include <kapp.h>
#include <kglobal.h>
#include <klocale.h>
#include <kconfig.h>
#include <kthemebase.h>
#include <kthemebase.h>
#include <kstddirs.h>
#include <kcharsets.h>
#include <kconfigbase.h>
#include <ksimpleconfig.h>
#include <kwm.h>
#include <kcolordlg.h>

#include "kresourceman.h"
#include "fonts.h"

#include "general.h"

#include <X11/Xlib.h>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>

#include "general.moc"

extern int dropError(Display *, XErrorEvent *);
int _getprop(Window w, Atom a, Atom type, long len, unsigned char **p);
bool getSimpleProperty(Window w, Atom a, long &result);

const char * KIconStyle::appName [] = {"kpanel", "kfm", "KDE"};
const int KIconStyle::nApp = 3;

KIconStyle::KIconStyle( QWidget *parent, QBoxLayout * topLayout )
{
    QString appTitle [] = { i18n("Panel"),
                            i18n("Konqueror"),
                            i18n("Other") };
    
    m_dictCBNormal.setAutoDelete(true);
    m_dictCBLarge.setAutoDelete(true);
    m_dictSettings.setAutoDelete(true);
    
    QGroupBox * gb = new QGroupBox ( i18n("Icon style"), parent );
    topLayout->addWidget( gb );
    
    // The layout containing the checkboxes
    QGridLayout * gLayout = new QGridLayout( gb, 1+nApp, 4, 10 /*autoborder*/);
    gLayout->setColStretch(3, 20); // avoid cb moving when resizing
    
    // The two labels on row 0
    QLabel * label = new QLabel( i18n("Normal"), gb );
    gLayout->addWidget( label, 0, 1 );
    
    label = new QLabel( i18n("Large"), gb );
    gLayout->addWidget( label, 0, 2 );
    
    // The label + 2 checkboxes on each row
    QRadioButton * cb;
    for (int i = 0 ; i < nApp ; i++) {
        QButtonGroup * group = new QButtonGroup( );
        label = new QLabel( appTitle[i], gb );
        gLayout->addWidget( label, i+1, 0 );
        
        cb = new QRadioButton( gb, "" );
        group->insert(cb);
        gLayout->addWidget( cb, i+1, 1 );
        m_dictCBNormal.insert( appName[i], cb ); // store the cb in the dict
        
        cb = new QRadioButton( gb, "" );
        group->insert(cb);
        gLayout->addWidget( cb, i+1, 2 );
        m_dictCBLarge.insert( appName[i], cb ); // store the cb in the dict
    }
}

KIconStyle::~KIconStyle()
{
}

void KIconStyle::apply()
{
    bool changed = false;
    for (int i = 0 ; i < nApp ; i++) {
        QString s = m_dictCBNormal[ appName[i] ] -> isChecked() ? "Normal" : "Large";
        // See if the settings have changed
        if (m_dictSettings [ appName[i] ] != s) {
            // Store new setting
            char * setting = new char [s.length()];
            strcpy( setting, s.ascii() );
            m_dictSettings.replace( appName[i], setting );
            // Apply it
            if ( strcmp( appName[i], "kpanel" ) == 0 )
                KWM::sendKWMCommand("kpanel:restart");
            else
                changed = true;
        }
    }
    if (changed)
        QMessageBox::information( 0L, i18n("Icons style"), i18n("The icon style change will not all be applied until you restart KDE."), i18n("OK"));
}

void KIconStyle::readSettings()
{
    KConfig *config = kapp->getConfig();
    
    KConfigGroupSaver saver(config, "KDE");
    for (int i = 0 ; i < nApp ; i++) {
        QString s = config->readEntry( QString(appName[i])+"IconStyle", "Normal" );
        m_dictCBNormal[ appName[i] ] -> setChecked( s == "Normal");
        m_dictCBLarge[ appName[i] ] -> setChecked( s == "Large" );
        char * setting = new char [s.length()];
        strcpy( setting, s.ascii() );
        m_dictSettings.insert( appName[i], setting ); // store initial value
    }    
}

void KIconStyle::writeSettings()
{
    KConfig *cfg = kapp->getConfig();
    
    KConfigGroupSaver saver(cfg, "KDE");
    for (int i = 0 ; i < nApp ; i++) {
        QString s = m_dictCBNormal[ appName[i] ] -> isChecked() ? "Normal" : "Large";
        cfg->writeEntry( QString(appName[i])+"IconStyle", s, 
                         true, true /* global setting */ );
        
        if (!strcmp(appName[i], "kpanel")) {
            KConfig * config = new KConfig("kpanelrc");
            config->setGroup("kpanel");
            // Special case for kpanel, as asked by Torsten :
            // Sync kpanel's size with icon size
            // Awful duplicated code from kcontrol/panel/panel.cpp
            // I will get killed by others developers...
            if (s == "Normal") {
                config->writeEntry("Style", "normal");
                config->writeEntry("BoxWidth",45);
                config->writeEntry("BoxHeight",45);
                config->writeEntry("Margin",0);
                config->writeEntry("TaskbarButtonHorizontalSize",4);
                config->writeEntry("DesktopButtonRows",2);
            } else {
                config->writeEntry("Style", "large");
                config->writeEntry("BoxWidth",52);
                config->writeEntry("BoxHeight",52);
                config->writeEntry("Margin",2);
                config->writeEntry("TaskbarButtonHorizontalSize",4);
                config->writeEntry("DesktopButtonRows",2);
            }
            config->sync();
            delete config;
        }
    }
    cfg->sync();
}

void KIconStyle::setDefaults()
{
    for (int i = 0 ; i < nApp ; i++) {
        m_dictCBNormal[ appName[i] ] -> setChecked( true );
        m_dictCBLarge[ appName[i] ] -> setChecked( false );
    }
}

// mosfet's style stuff 04/26/99 (mosfet)
KThemeListBox::KThemeListBox(QWidget *parent, const char *name)
    : QListView(parent, name)
{
    addColumn(i18n("Name:"));
    addColumn(i18n("Description:"));
    setAllColumnsShowFocus(true);
    KGlobal::dirs()->addResourceType("themes", KStandardDirs::kde_default("data") + "/kstyle/themes");
    QStringList list = KGlobal::dirs()->getResourceDirs("themes");
    for (QStringList::ConstIterator it = list.begin(); it != list.end(); it++)
        readThemeDir(*it);

    if (!currentItem())
      setSelected(firstChild(), true);
}

void KThemeListBox::readThemeDir(const QString &directory)
{
    QString name, desc;

    KConfig *kconfig = KGlobal::config();
    QString oldGroup = kconfig->group();
    kconfig->setGroup("Misc");
    curName = kconfig->readEntry("Name", "");
    kconfig->setGroup(oldGroup);
    
    QDir dir(directory, "*.themerc");
    if(dir.exists()){
        const QFileInfoList *list = dir.entryInfoList();
        QFileInfoListIterator it(*list);
        QFileInfo *fi;
        while((fi = it.current())){
            KSimpleConfig config(fi->absFilePath(), true);
            config.setGroup("Misc");
            name = config.readEntry("Name", fi->baseName());
            desc = config.readEntry("Comment", i18n("No description available."));
            QListViewItem* item = new QListViewItem(this, name, desc, fi->absFilePath());
            if(name == curName) {
                setSelected(item, true);
                ensureItemVisible(item);
            }
            ++it;
        }
    }
}

QString KThemeListBox::currentName()
{
    return(curName);
}

QString KThemeListBox::currentFile()
{
    return (currentItem() ? currentItem()->text(2) : QString::null);
}

void KThemeListBox::apply()
{
    writeSettings();
}

void KThemeListBox::writeSettings()
{
    if(currentItem()->text(0) == curName)
        return;

    KThemeBase::applyConfigFile(currentItem()->text(2));
}

//------------------------------------------------------------------

KGeneral::KGeneral( QWidget *parent, int mode, int desktop )
	: KDisplayModule( parent, mode, desktop )
{
	changed = false;
	useRM = true;
	macStyle = false;
    
	// if we are just initialising we don't need to create setup widget
	if ( mode == Init )
		return;
	
	kde_display = x11Display();
	KDEChangeGeneral = XInternAtom( kde_display, "KDEChangeGeneral", False);
	screen = DefaultScreen(kde_display);
	root = RootWindow(kde_display, screen);
    
    setName( i18n("Style") );
    
	readSettings();
	
	QBoxLayout *topLayout = new QVBoxLayout( this, 10 );
    
    // my little style list (mosfet 04/26/99)
    QBoxLayout *lay = new QHBoxLayout( 10);
    QGroupBox *themeBox = new QGroupBox(i18n("Widget style and theme:"), this);
    lay->addWidget(themeBox);
    lay->setStretchFactor(themeBox, 1);
    
    QBoxLayout *themeLayout = new QVBoxLayout(themeBox, 10);
    
    themeList = new KThemeListBox(themeBox);
    themeList->setMinimumSize(QSize(100,100));
    themeLayout->addSpacing(10);
    themeLayout->addWidget(themeList);
    connect(themeList, SIGNAL(currentChanged(QListViewItem*)),
            SLOT(slotChangeStylePlugin(QListViewItem*)));

    iconStyle = new KIconStyle( this, lay );
	iconStyle->readSettings();

    topLayout->addLayout(lay);

    styles = new QButtonGroup ( i18n( "Other settings for drawing:" ), this );
	topLayout->addWidget(styles, 10);
    
	QBoxLayout *vlay = new QVBoxLayout (styles, 10);
	vlay->addSpacing(10);
    
	cbMac = new QCheckBox( i18n( "&Menubar on top of the screen in "
                                 "the style of MacOS" ), styles);
    cbMac->setChecked( macStyle );
	connect( cbMac, SIGNAL( clicked() ), SLOT( slotMacStyle()  )  );
	vlay->addWidget( cbMac, 10 );

	cbRes = new QCheckBox( i18n( "&Apply fonts and colors to non-KDE apps" ),
                           styles);
    cbRes->setChecked( useRM );
	connect( cbRes, SIGNAL( clicked() ), SLOT( slotUseResourceManager()  )  );
	vlay->addWidget( cbRes, 10 );

	tbStyle = new QButtonGroup( i18n( "Style options for toolbars:" ), this);
	topLayout->addWidget(tbStyle, 10);

	vlay = new QVBoxLayout( tbStyle, 10 );
	vlay->addSpacing( 10 );
	
	lay = new QHBoxLayout( 10 );
	vlay->addLayout( lay );

	tbIcon   = new QRadioButton( i18n( "&Icons only" ), tbStyle);
	tbText   = new QRadioButton( i18n( "&Text only" ), tbStyle);
	tbAside  = new QRadioButton( i18n( "Text a&side icons" ), tbStyle);
	tbUnder  = new QRadioButton( i18n( "Text &under icons" ), tbStyle);
    
	tbHilite = new QCheckBox( i18n( "&Highlight buttons under mouse" ), 
                              tbStyle);
	tbTransp = new QCheckBox( i18n( "Tool&bars are transparent when"
                                    " moving" ), tbStyle);

    switch(tbUseText) {
    case 0:
        tbIcon->setChecked(true);
        break;
    case 1:
        tbAside->setChecked(true);
        break;
    case 2:
        tbText->setChecked(true);
        break;
    case 3:
        tbUnder->setChecked(true);
        break;
    default:
        tbIcon->setChecked(true);
    }
    
	connect( tbIcon , SIGNAL( clicked() ), SLOT( slotChangeTbStyle()  )  );
	connect( tbText , SIGNAL( clicked() ), SLOT( slotChangeTbStyle()  )  );
	connect( tbAside, SIGNAL( clicked() ), SLOT( slotChangeTbStyle()  )  );
	connect( tbUnder, SIGNAL( clicked() ), SLOT( slotChangeTbStyle()  )  );

    tbHilite->setChecked( tbUseHilite );
    tbTransp->setChecked( tbMoveTransparent );
    
	lay->addWidget(tbIcon, 10);
	lay->addWidget(tbText, 10);
	lay->addWidget(tbAside, 10);
	lay->addWidget(tbUnder, 10);
    
	vlay->addWidget(tbHilite, 10);
	vlay->addWidget(tbTransp, 10);
    
	topLayout->addStretch( 100 );
}

void KGeneral::slotChangeStylePlugin(QListViewItem*)
{
    changed=true;
}

void KGeneral::slotChangeTbStyle()
{
  if (tbIcon->isChecked() )
      tbUseText = 0;
  else if (tbText->isChecked() )
      tbUseText = 2;
  else if (tbAside->isChecked() )
      tbUseText = 1;
  else if (tbUnder->isChecked() )
      tbUseText = 3;
  else
      tbUseText = 0 ;
}

void KGeneral::slotUseResourceManager()
{
	useRM = cbRes->isChecked();
		
	changed=true;
}

void KGeneral::slotMacStyle()
{
	macStyle = cbMac->isChecked();
		
	changed=true;
}

KGeneral::~KGeneral()
{
}

void KGeneral::readSettings( int )
{		
	QString str;
    
	KConfig *config = KGlobal::config();
	KConfigGroupSaver saver(config, "KDE");

	str = config->readEntry( "widgetStyle", "Platinum" );
	if ( str == "Platinum" )
        applicationStyle = WindowsStyle;
	else if ( str == "Windows 95" )
        applicationStyle = WindowsStyle;
	else
        applicationStyle = MotifStyle;
    
	macStyle = config->readBoolEntry( "macStyle", false);

	config->setGroup( "Toolbar style" );

	tbUseText = config->readNumEntry( "IconText", 0);
	int val = config->readNumEntry( "Highlighting", 1);
	tbUseHilite = val? true : false;
	tbMoveTransparent = config->readBoolEntry( "TransparentMoving", true);
			
	config->setGroup("X11");
	useRM = config->readBoolEntry( "useResourceManager", true );
}

void KGeneral::setDefaults()
{
	cbRes->setChecked( true );
	cbMac->setChecked( false );
	useRM = true;
	macStyle = false;
	slotMacStyle();
	iconStyle->setDefaults();
    
	tbUseText = 0;
	tbUseHilite = true;
	tbMoveTransparent = true;
	tbIcon->setChecked( true );
	tbHilite->setChecked( true );
	tbTransp->setChecked( true );
}

void KGeneral::defaultSettings()
{
	setDefaults();
}

void KGeneral::writeSettings()
{
	iconStyle->writeSettings();
    themeList->writeSettings();
    if ( !changed )
		return;
		
	KConfig *config = kapp->getConfig();
	KConfigGroupSaver saver(config, "KDE");
    
	QString str;
        
	config->writeEntry( "macStyle", macStyle);

	config->setGroup( "Toolbar style" );
	config->writeEntry( "IconText", tbUseText);
	config->writeEntry( "Highlighting", (int)tbUseHilite);
	config->writeEntry( "TransparentMoving", (int)tbMoveTransparent);

	config->setGroup("X11");
	config->writeEntry( "useResourceManager", useRM );

	config->sync();
	
	if ( useRM ) {
		QApplication::setOverrideCursor( waitCursor );
        
		// the krdb run is for colors and other parameters (Matthias)
		KProcess proc;
		proc.setExecutable("krdb");
		proc.start( KProcess::Block );
        
		// still some KResourceMan stuff stuff. We need to
		// clean this up.  The best thing would be to use
		// KResourceMan always for KDE applications to make
		// the desktop settings machine independent but
		// per-display (Matthias)
        KResourceMan *krdb = new KResourceMan();
        
        krdb->sync();
        
		QApplication::restoreOverrideCursor();
	}
	
    KWM::sendKWMCommand((macStyle ? "macStyleOn" : "macStyleOff"));
    
	QApplication::syncX();
}

void KGeneral::slotApply()
{
	writeSettings();
	apply();
}

int _getprop(Window w, Atom a, Atom type, long len, unsigned char **p)
{
    Atom real_type;
    int format;
    unsigned long n, extra;
    int status;
    
    status = XGetWindowProperty(qt_xdisplay(), w, a, 0L, len, False, type, &real_type, &format, &n, &extra, p);
    if (status != Success || *p == 0)
        return -1;
    if (n == 0)
        XFree((char*) *p);
    return n;
}

bool getSimpleProperty(Window w, Atom a, long &result)
{
    long *p = 0;
    
    if (_getprop(w, a, a, 1L, (unsigned char**)&p) <= 0){
        return false;
    }
    
    result = p[0];
    XFree((char *) p);
    return true;
}

void KGeneral::apply( bool  )
{
    iconStyle->apply(); 
    themeList->apply(); 
	if ( !changed )
		return;
	
	XEvent ev;
	unsigned int i, nrootwins;
	Window dw1, dw2, *rootwins;
	int (*defaultHandler)(Display *, XErrorEvent *);
    
	defaultHandler=XSetErrorHandler(dropError);
	
	XQueryTree(kde_display, root, &dw1, &dw2, &rootwins, &nrootwins);
	
	Atom a = XInternAtom(qt_xdisplay(), "KDE_DESKTOP_WINDOW", False);
	for (i = 0; i < nrootwins; i++) {
        long result = 0;
        getSimpleProperty(rootwins[i],a, result);
        if (result) {
            ev.xclient.type = ClientMessage;
            ev.xclient.display = kde_display;
            ev.xclient.window = rootwins[i];
            ev.xclient.message_type = KDEChangeGeneral;
            ev.xclient.format = 32;
            
            XSendEvent(kde_display, rootwins[i] , False, 0L, &ev);
        }
	}
    
	XFlush(kde_display);
	XSetErrorHandler(defaultHandler);
	
	XFree((char *) rootwins);
	
	changed=false;
}


void KGeneral::slotHelp()
{
	kapp->invokeHTMLHelp( "kcmdisplay/index-7.html", "" );
}

void KGeneral::applySettings()
{
    writeSettings();
    apply( true );
}
