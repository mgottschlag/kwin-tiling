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

#include "kresourceman.h"

#include "general.h"    

#include <kapp.h>
#include <kglobal.h>
#include <kthemebase.h>
#include <kcharsets.h>
#include <kconfigbase.h>
#include <ksimpleconfig.h>
#include <kwm.h>
#include <kcolordlg.h>

#include <X11/Xlib.h>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <klocale.h>
#include <kconfig.h>
#include <kthemebase.h>

#include "general.moc"

extern int dropError(Display *, XErrorEvent *);

FontUseItem::FontUseItem( const QString& n, QFont default_fnt, bool f )
	: selected(0)
{
	_text = n;
	_default = _font = default_fnt;
	fixed = f;
}

QString FontUseItem::fontString( QFont rFont )
{
	QString aValue;
	aValue = rFont.rawName();
	return aValue;
	QFontInfo fi( rFont );
	
	aValue = "-*-";
	aValue += fi.family();
	
	if ( fi.bold() )
		aValue += "-bold";
	else
		aValue += "-medium";
	
	if ( fi.italic() )
		aValue += "-i";
	else
		aValue += "-r";
		
	//aValue += "-normal-*-*-";
	//	
	//QString s = QString( "%1-*-*-*-*-").arg(fi.pointSize()*10 );
	//aValue += s;
	
	aValue += "-normal-*-";
		
	QString s = QString( "%1-*-*-*-*-*-").arg(fi.pointSize() );
	aValue += s;
	
	
	switch ( fi.charSet() ) {
		case QFont::Latin1:
			aValue += "iso8859-1";
			break;
		case QFont::AnyCharSet:
		default:
			aValue += "-*";
			break;
		case QFont::ISO_8859_2:
			aValue += "iso8859-2";
			break;
		case QFont::ISO_8859_3:
			aValue += "iso8859-3";
			break;
		case QFont::ISO_8859_4:
			aValue += "iso8859-4";
			break;
		case QFont::ISO_8859_5:
			aValue += "iso8859-5";
			break;
		case QFont::ISO_8859_6:
			aValue += "iso8859-6";
			break;
		case QFont::ISO_8859_7:
			aValue += "iso8859-7";
			break;
		case QFont::ISO_8859_8:
			aValue += "iso8859-8";
			break;
		case QFont::ISO_8859_9:
			aValue += "iso8859-9";
			break;
	}
	return aValue;
}

void FontUseItem::setRC( const QString& group, const QString& key, const QString&rc )
{
	_rcgroup = group;
	_rckey = key;
	if ( !rc.isNull() ) _rcfile = rc;
}

void FontUseItem::setDefault()
{
	_font = _default;
}

void FontUseItem::readFont()
{
	KConfigBase *config=NULL;

	if ( _rcfile.isEmpty() ) {
		config  = kapp->getConfig();
	} else {
		QString s( KApplication::localconfigdir() );
		s += "/";
		s += _rcfile;
		config = new KSimpleConfig( s, true );
	}

	config->setGroup( _rcgroup );
	
	QFont tmpFnt( _font );
	_font = config->readFontEntry( _rckey, &tmpFnt );
}

void FontUseItem::writeFont()
{
	KConfigBase *config;
	if ( _rcfile.isEmpty() ) {
		config  = kapp->getConfig();
	} else {
		QString s( KApplication::localconfigdir() );
		s += "/";
		s += _rcfile;
		config = new KSimpleConfig( s );
	}
	
	config->setGroup( _rcgroup );
	if ( _rcfile.isEmpty() ) {
		 config->writeEntry( _rckey, _font, true, true );
	} else {
		config->writeEntry( _rckey, _font );
	}
	
	 config->sync();
	
	//delete config;
}

const char * KIconStyle::appName [] = {"kpanel", "kfm", "KDE"};
const int KIconStyle::nApp = 3;

KIconStyle::KIconStyle( QWidget *parent, QBoxLayout * topLayout )
{
    QString appTitle [] = { i18n("Panel"),
                            i18n("File manager and desktop icons"),
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
    label->setMinimumSize(label->sizeHint());
    gLayout->addWidget( label, 0, 1 );

    label = new QLabel( i18n("Large"), gb );
    label->setMinimumSize(label->sizeHint());
    gLayout->addWidget( label, 0, 2 );

    // The label + 2 checkboxes on each row
    QRadioButton * cb;
    for (int i = 0 ; i < nApp ; i++)
    {
        QButtonGroup * group = new QButtonGroup( );
        label = new QLabel( appTitle[i], gb );
        label->setMinimumSize(label->sizeHint());
        gLayout->addWidget( label, i+1, 0 );

        cb = new QRadioButton( gb, "" );
        group->insert(cb);
        cb->setMinimumSize(cb->sizeHint());
        gLayout->addWidget( cb, i+1, 1 );
        m_dictCBNormal.insert( appName[i], cb ); // store the cb in the dict

        cb = new QRadioButton( gb, "" );
        group->insert(cb);
        cb->setMinimumSize(cb->sizeHint());
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
    for (int i = 0 ; i < nApp ; i++)
    {
        QString s = m_dictCBNormal[ appName[i] ] -> isChecked() ? "Normal" : "Large";
        // See if the settings have changed
        if (m_dictSettings [ appName[i] ] != s)
        {
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
      QMessageBox::information( 0L, i18n("Icons style"), i18n("The icon style change will not all be applied until you restart KDE."));
}

void KIconStyle::readSettings()
{
    KConfig *config = kapp->getConfig();
    
    KConfigGroupSaver saver(config, "KDE");
    for (int i = 0 ; i < nApp ; i++)
    {
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
    for (int i = 0 ; i < nApp ; i++)
    {
        QString s = m_dictCBNormal[ appName[i] ] -> isChecked() ? "Normal" : "Large";
        cfg->writeEntry( QString(appName[i])+"IconStyle", s, 
                            true, true /* global setting (share/config/kdeglobals) */ );
        if (!strcmp(appName[i], "kpanel"))
        {
          KConfig * config = new KConfig(KApplication::kde_configdir() + "/kpanelrc",
                                         KApplication::localconfigdir() + "/kpanelrc");
          config->setGroup("kpanel");
          // Special case for kpanel, as asked by Torsten :
          // Sync kpanel's size with icon size
          // Awful duplicated code from kcontrol/panel/panel.cpp
          // I will get killed by others developers...
          if (s == "Normal")
          {
            config->writeEntry("Style", "normal");
            config->writeEntry("BoxWidth",45);
            config->writeEntry("BoxHeight",45);
            config->writeEntry("Margin",0);
            config->writeEntry("TaskbarButtonHorizontalSize",4);
            //config->writeEntry("DesktopButtonFont","*-helvetica-medium-r-normal--12-*");
            config->writeEntry("DesktopButtonRows",2);
            //config->writeEntry("DateFont","*-times-medium-i-normal--12-*");
          } else {
            config->writeEntry("Style", "large");
            config->writeEntry("BoxWidth",52);
            config->writeEntry("BoxHeight",52);
            config->writeEntry("Margin",2);
            config->writeEntry("TaskbarButtonHorizontalSize",4);
            //config->writeEntry("DesktopButtonFont","*-helvetica-medium-r-normal--14-*");
            config->writeEntry("DesktopButtonRows",2);
            //config->writeEntry("DateFont","*-times-bold-i-normal--12-*");
          }
          config->sync();
          delete config;
        }
    }
    cfg->sync();
}

void KIconStyle::setDefaults()
{
    for (int i = 0 ; i < nApp ; i++)
    {
        m_dictCBNormal[ appName[i] ] -> setChecked( true );
        m_dictCBLarge[ appName[i] ] -> setChecked( false );
    }
}

// mosfet's style stuff 04/26/99 (mosfet)
KThemeListBox::KThemeListBox(QWidget *parent, const char *name)
    : KTabListBox(parent, name, 2)
{
    setColumn(0, i18n("Name:"), 100);
    setColumn(1, i18n("Description:"), 250);
    setSeparator('\t');
    readThemeDir(kapp->localkdedir()+"/share/apps/kstyle/themes");
    readThemeDir(kapp->kde_datadir()+"/kstyle/themes");
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
            KSimpleConfig config(fi->absFilePath());
            config.setGroup("Misc");
            name = config.readEntry("Name", fi->baseName());
            desc = config.readEntry("Comment",
                                    i18n("No description available."));
            insertItem(name + "\t" + desc);
            fileList.append(fi->absFilePath().ascii());
            if(name == curName)
                setCurrentItem(count()-1);
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
    return((currentItem() != -1) ? QString(fileList.at(currentItem())) :
           QString::null);
}

void KThemeListBox::apply()
{
    writeSettings();
}

void KThemeListBox::writeSettings()
{
    if(text(currentItem(), 0) == curName)
        return;
    
    KThemeBase::applyConfigFile(fileList.at(currentItem()));
}

//------------------------------------------------------------------

KGeneral::KGeneral( QWidget *parent, int mode, int desktop )
	: KDisplayModule( parent, mode, desktop )
{
	changed = false;
	useRM = true;
	macStyle = false;//CT

	//debug("KGeneral::KGeneral");
	
	// if we are just initialising we don't need to create setup widget
	if ( mode == Init )
		return;
	
	kde_display = x11Display();
	KDEChangeGeneral = XInternAtom( kde_display, "KDEChangeGeneral", False);
	screen = DefaultScreen(kde_display);
	root = RootWindow(kde_display, screen);

        setName( i18n("Style").ascii() );

	readSettings();
	
	QBoxLayout *topLayout = new QVBoxLayout( this, 10 );

        // my little style list (mosfet 04/26/99)
        QGroupBox *themeBox = new QGroupBox(i18n("Widget style and theme:"),
                                            this);
        topLayout->addWidget(themeBox);
        QBoxLayout *themeLayout = new QVBoxLayout(themeBox, 10);
        themeList = new KThemeListBox(themeBox);
        themeList->setMinimumSize(QSize(100,100));
        themeLayout->addSpacing(10);
        themeLayout->addWidget(themeList);
        connect(themeList, SIGNAL(highlighted(int, int)),
                                  SLOT(slotChangeStylePlugin(int, int)));
        
        styles = new QButtonGroup ( i18n( "Other settings for drawing:" ),
				    this );
	topLayout->addWidget(styles, 10);

	QBoxLayout *vlay = new QVBoxLayout (styles, 10);
	vlay->addSpacing(10);

	//CT 30Nov1998
	cbMac = new QCheckBox( i18n( "&Menubar on top of the screen in "
				     "the style of MacOS" ), styles);
	//CT	cbMac->adjustSize();
	//CT	cbMac->setMinimumSize(cbMac->size());

	if( macStyle )
	    cbMac->setChecked( true );
	else
	    cbMac->setChecked( false);
	
	connect( cbMac, SIGNAL( clicked() ), SLOT( slotMacStyle()  )  );
	
	vlay->addWidget( cbMac, 10 );

	cbRes = new QCheckBox( i18n( "&Apply fonts and colors to non-KDE "
				     "apps" ), styles);
	//CT	cbRes->setMinimumSize(cbRes->sizeHint());

	if( useRM )
	        cbRes->setChecked( true );
	else
		cbRes->setChecked( false);
	
	connect( cbRes, SIGNAL( clicked() ), SLOT( slotUseResourceManager()  )  );
	
	vlay->addWidget( cbRes, 10 );//CT

	iconStyle = new KIconStyle( this, topLayout ); // DF


	iconStyle->readSettings(); // DF

	//CT 04Apr1999 - toolbars styles
	tbStyle = new QButtonGroup( i18n( "Style options for toolbars:" ),
				    this);
	topLayout->addWidget(tbStyle, 10);

	//CT 05Apr1999
	vlay = new QVBoxLayout( tbStyle, 10 );

	vlay->addSpacing( 10 );
	
	QBoxLayout *lay = new QHBoxLayout( 10 );
	vlay->addLayout( lay );

	tbIcon   = new QRadioButton( i18n( "&Icons only" ), tbStyle);
	tbText   = new QRadioButton( i18n( "&Text only" ), tbStyle);
	tbAside  = new QRadioButton( i18n( "Text a&side icons" ), tbStyle);
	tbUnder  = new QRadioButton( i18n( "Text &under icons" ), tbStyle);

	tbHilite = new QCheckBox( i18n( "&Highlight buttons under mouse" ), 
				tbStyle);
	tbTransp = new QCheckBox( i18n( "Tool&bars are transparent when"
					" moving" ), tbStyle);
	
	if (tbUseText == 0) 
	  tbIcon->setChecked( true );
	else if (tbUseText == 1)
	  tbAside->setChecked( true );
	else if (tbUseText == 2)
	  tbText->setChecked( true );
	else if (tbUseText == 3)
	  tbUnder->setChecked( true );
	else
	  tbIcon->setChecked( true );

	connect( tbIcon , SIGNAL( clicked() ), SLOT( slotChangeTbStyle()  )  );
	connect( tbText , SIGNAL( clicked() ), SLOT( slotChangeTbStyle()  )  );
	connect( tbAside, SIGNAL( clicked() ), SLOT( slotChangeTbStyle()  )  );
	connect( tbUnder, SIGNAL( clicked() ), SLOT( slotChangeTbStyle()  )  );

	//CT

	if (tbUseHilite)
	  tbHilite->setChecked( true );
	else
	  tbHilite->setChecked( false );

	if (tbMoveTransparent)
	  tbTransp->setChecked( true );
	else
	  tbTransp->setChecked( false );

	lay->addWidget(tbIcon, 10);
	lay->addWidget(tbText, 10);
	lay->addWidget(tbAside, 10);
	lay->addWidget(tbUnder, 10);

	vlay->addWidget(tbHilite, 10);
	vlay->addWidget(tbTransp, 10);

	topLayout->addStretch( 100 );
	//CT topLayout->activate(); //CT not needed for Qt-2.0

}

void KGeneral::slotChangeStylePlugin(int, int)
{
    changed=true;
}

//CT 05Apr 1999
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

//CT 30Nov1998 - mac style set
void KGeneral::slotMacStyle()
{
	macStyle = cbMac->isChecked();
		
	changed=true;
}
//CT

KGeneral::~KGeneral()
{
	delete iconStyle; // DF
}



void KGeneral::readSettings( int )
{		
	QString str;

	KConfig *config = KGlobal::config();
	KConfigGroupSaver saver(config, "KDE");

        // This doesn't do anything anymore (mosfet)
        //CT 04Apr1999
	str = config->readEntry( "widgetStyle", "Platinum" );
	if ( str == "Platinum" )
	  //CT 04Apr1999 - for the moment Qt doesn't have a PlatinumStyle 
	  //CT    identifier and however kapp is "frozen" on Platinum style
	  applicationStyle = WindowsStyle;
	else if ( str == "Windows 95" )
	  applicationStyle = WindowsStyle;
	else
	  applicationStyle = MotifStyle;

	//CT 30Nov1998 - mac style set

	str = config->readEntry( "macStyle", "off");
	if ( str == "on" )
	  macStyle = true;
	else
	  macStyle = false;
	//CT

	//CT 04Apr1999 - read toolbar style
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
  //CT        cbStyle->setChecked( true );
	cbRes->setChecked( true );
	cbMac->setChecked( false );//CT
	useRM = true;
	macStyle = false;//CT
	slotMacStyle();//CT
	iconStyle->setDefaults(); // DF

	//CT 04Apr1999
	tbUseText = 0;
	tbUseHilite = true;
	tbMoveTransparent = true;
	tbIcon->setChecked( true );
	tbHilite->setChecked( true );
	tbTransp->setChecked( true );
	//CT
}

void KGeneral::defaultSettings()
{
	setDefaults();
}

void KGeneral::writeSettings()
{
	iconStyle->writeSettings(); // DF
        themeList->writeSettings();
        if ( !changed )
		return;
		
	KConfig *config = kapp->getConfig();
	KConfigGroupSaver saver(config, "KDE");

	QString str;
        
	//CT 30Nov1998 - mac style set
	config->writeEntry( "macStyle", macStyle?"on":"off", true, true);
	//CT

	//CT 05Apr 1999
	// bugfix PGB 05/28/1999
	config->setGroup( "Toolbar style" );

	config->writeEntry( "IconText", tbUseText);
	config->writeEntry( "Highlighting", tbUseHilite?1:0);
	config->writeEntry( "TransparentMoving", tbMoveTransparent?1:0);
	//CT

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

                // KThemeListBox sets this now (mosfet)

                // krdb->setGroup( "KDE" );
                // krdb->writeEntry( "widgetStyle", str );

                krdb->sync();

		QApplication::restoreOverrideCursor();
	}
	

	if (macStyle) {
	    KWM::sendKWMCommand("macStyleOn");
	}
	else {
	    KWM::sendKWMCommand("macStyleOff");
	}
	QApplication::syncX();
	

}

void KGeneral::slotApply()
{
	writeSettings();
	apply();
}

//Matthias
static int _getprop(Window w, Atom a, Atom type, long len, unsigned char **p){
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

//Matthias
static bool getSimpleProperty(Window w, Atom a, long &result){
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
        iconStyle->apply(); // DF
        themeList->apply(); // MF ;-)
	if ( !changed )
		return;
	
	XEvent ev;
	unsigned int i, nrootwins;
	Window dw1, dw2, *rootwins;
	int (*defaultHandler)(Display *, XErrorEvent *);


	defaultHandler=XSetErrorHandler(dropError);
	
	XQueryTree(kde_display, root, &dw1, &dw2, &rootwins, &nrootwins);
	
	// Matthias
	Atom a = XInternAtom(qt_xdisplay(), "KDE_DESKTOP_WINDOW", False);
	for (i = 0; i < nrootwins; i++) {
	  long result = 0;
	  getSimpleProperty(rootwins[i],a, result);
	  if (result){
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
	kapp->invokeHTMLHelp( "kcmdisplay/kdisplay-7.html", "" );
}

void KGeneral::applySettings()
{
  writeSettings();
  apply( true );
}





//------------------------------------------------------------------

KFonts::KFonts( QWidget *parent, int mode, int desktop )
	: KDisplayModule( parent, mode, desktop )
{
	int i;
	changed = false;

	//debug("KFonts::KFonts");
	
	// if we are just initialising we don't need to create setup widget
	if ( mode == Init )
		return;
	
	kde_display = x11Display();
	KDEChangeGeneral = XInternAtom( kde_display, "KDEChangeGeneral", False);
	screen = DefaultScreen(kde_display);
	root = RootWindow(kde_display, screen);

	setName( i18n("Fonts").ascii() );

	readSettings();
	
	QBoxLayout *topLayout = new QVBoxLayout( this, 10 );
	
	topLayout->addSpacing( 15 );
	
	QBoxLayout *pushLayout = new QHBoxLayout( 5 );
	
	topLayout->addLayout( pushLayout );
	
	FontUseItem *item = new FontUseItem( i18n("General font"),
				QFont( "helvetica", 12 ) );
	item->setRC( "General", "font" );
	fontUseList.append( item );
	
	item = new FontUseItem( i18n("Fixed font"),
				QFont( "fixed", 10 ), true );
	item->setRC( "General", "fixedFont" );
	fontUseList.append( item );
	
	//CT 03Nov1998 - this code was here already. Only reactivated
	item = new FontUseItem( i18n("Window title font"),
				QFont( "helvetica", 12, QFont::Bold ) );
	item->setRC( "WM", "titleFont" );
	fontUseList.append( item );
				
	item = new FontUseItem( i18n("Panel button font"),
				QFont( "helvetica", 12 )  );
	item->setRC( "kpanel", "DesktopButtonFont", "kpanelrc" );
	fontUseList.append( item );
	
	item = new FontUseItem( i18n("Panel clock font"),
				QFont( "helvetica", 12, QFont::Normal) );
	item->setRC( "kpanel", "DateFont", "kpanelrc" );
	fontUseList.append( item );
	
	for ( i = 0; i < (int) fontUseList.count(); i++ )
		fontUseList.at( i )->readFont();
	
	lbFonts = new QListBox( this );
	for ( i=0; i < (int) fontUseList.count(); i++ )
  	     lbFonts->insertItem( fontUseList.at( i )->text() );
	lbFonts->adjustSize();
	lbFonts->setMinimumSize(lbFonts->size());
	
	connect( lbFonts, SIGNAL( highlighted( int ) ),
		 SLOT( slotPreviewFont( int ) ) );
			
	pushLayout->addWidget( lbFonts );
	
	fntChooser = new KFontChooser( this );
	
	connect( fntChooser, SIGNAL( fontSelected(const QFont &) ), this,
		SLOT( slotSetFont(const QFont &) ) );
	
	pushLayout->addWidget( fntChooser );
	
	QBoxLayout *stackLayout = new QVBoxLayout( 4 );
	
	topLayout->addLayout( stackLayout );

	QLabel *label = new QLabel( i18n("Sample text"), this );
	label->adjustSize();
	label->setFixedHeight( label->height() );
	label->setMinimumWidth(label->width());

	stackLayout->addWidget( label );
	
	lbFonts->setCurrentItem( 0 );

	topLayout->activate();
}



KFonts::~KFonts()
{
}



void KFonts::readSettings( int )
{		
    useRM = kapp->getConfig()->readBoolEntry( "useResourceManager", true );
}

void KFonts::setDefaults()
{
	int ci = lbFonts->currentItem();
	for ( int i = 0; i < (int) fontUseList.count(); i++ )
		fontUseList.at( i )->setDefault();
	fontUseList.at( ci );
	slotPreviewFont( ci );
	
}

void KFonts::defaultSettings()
{
	setDefaults();
}

void KFonts::writeSettings()
{
	if ( !changed )
		return;
		
	for ( int i = 0; i < (int) fontUseList.count(); i++ )
		fontUseList.at( i )->writeFont();	
	

	if ( useRM ) {
		QApplication::setOverrideCursor( waitCursor );

		KResourceMan *krdb = new KResourceMan();
		for ( int i = 0; i < (int) fontUseList.count(); i++ ) {
			FontUseItem *it = fontUseList.at( i );
			if ( !it->rcFile() ) {
				krdb->setGroup( it->rcGroup() );
				krdb->writeEntry( it->rcKey(), it->font() );
			}
		}
		krdb->sync();

		QApplication::restoreOverrideCursor();
	}
	
	fontUseList.at( lbFonts->currentItem() );
	
}

void KFonts::slotApply()
{
	writeSettings();
	apply();
}


void KFonts::apply( bool  )
{
	if ( !changed )
		return;
	
	XEvent ev;
	unsigned int i, nrootwins;
	Window dw1, dw2, *rootwins;
	int (*defaultHandler)(Display *, XErrorEvent *);


	defaultHandler=XSetErrorHandler(dropError);
	
	XQueryTree(kde_display, root, &dw1, &dw2, &rootwins, &nrootwins);
	
	// Matthias
	Atom a = XInternAtom(qt_xdisplay(), "KDE_DESKTOP_WINDOW", False);
	for (i = 0; i < nrootwins; i++) {
	  long result = 0;
	  getSimpleProperty(rootwins[i],a, result);
	  if (result){
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

void KFonts::slotSetFont(const QFont &fnt)
{
	fontUseList.current()->setFont( fnt );
	changed = true;
}

void KFonts::slotPreviewFont( int index )
{
	fntChooser->setFont( fontUseList.at( index )->font(),
			fontUseList.at( index )->spacing() );
}

void KFonts::slotHelp()
{
	kapp->invokeHTMLHelp( "kcmdisplay/kdisplay-6.html", "" );
}

void KFonts::applySettings()
{
  writeSettings();
  apply( true );
}
