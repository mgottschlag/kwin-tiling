    /*

    Greeter widget for kdm

    Copyright (C) 1997, 1998, 2000 Steffen Hansen <hansen@kde.org>
    Copyright (C) 2000-2003 Oswald Buddenhagen <ossi@kde.org>


    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    */

#include "kgreeter.h"
#include "kdmconfig.h"
#include "kdmclock.h"
#include "kdm_greet.h"
#include "kdm_config.h"

#include <kapplication.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kseparator.h>
#include <klistview.h>
#include <ksimpleconfig.h>

#undef Unsorted // x headers suck - make qdir.h work with --enable-final
#include <qdir.h>
#include <qfile.h>
#include <qimage.h>
#include <qpopupmenu.h>
#include <qtimer.h>
#include <qheader.h>
#include <qstyle.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qpushbutton.h>

#include <pwd.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

#include <X11/Xlib.h>

class UserListView : public KListView {
public:
    UserListView( QWidget* parent = 0, const char *name = 0 )
	: KListView( parent, name )
	, cachedSizeHint( -1, 0 )
    {
	setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Ignored );
	header()->hide();
	addColumn( QString::null );
	setColumnAlignment( 0, AlignVCenter );
	setResizeMode( QListView::LastColumn );
    }

    mutable QSize cachedSizeHint;

protected:
    virtual QSize sizeHint() const
    {
	if (!cachedSizeHint.isValid()) {
	    constPolish();
	    uint maxw = 0;
	    for (QListViewItem *itm = firstChild(); itm; itm = itm->nextSibling()) {
		uint thisw = itm->width( fontMetrics(), this, 0 );
		if (thisw > maxw)
		    maxw = thisw;
	    }
	    cachedSizeHint.setWidth(
		style().pixelMetric( QStyle::PM_ScrollBarExtent ) +
		frameWidth() * 2 + maxw );
	}
	return cachedSizeHint;
    }
};


int KGreeter::curPlugin = -1;
PluginList KGreeter::pluginList;

KGreeter::KGreeter()
  : inherited()
  , dName( dname )
  , userView( 0 )
  , clock( 0 )
  , pixLabel( 0 )
  , nNormals( 0 )
  , nSpecials( 0 )
  , curPrev( -1 )
  , curSel( -1 )
  , prevValid( true )
  , needLoad( false )
{
    stsFile = new KSimpleConfig( kdmcfg->_stsFile );
    stsFile->setGroup( "PrevUser" );

    QGridLayout* main_grid = new QGridLayout( winFrame, 4, 2, 10 );
    QBoxLayout* hbox1 = new QHBoxLayout( 10 );
    QBoxLayout* hbox2 = new QHBoxLayout( 10 );

    grid = new QGridLayout( 5, 4, 5 );

    if (!kdmcfg->_greetString.isEmpty()) {
	QLabel* welcomeLabel = new QLabel( kdmcfg->_greetString, winFrame );
	welcomeLabel->setAlignment( AlignCenter );
	welcomeLabel->setFont( kdmcfg->_greetFont );
	main_grid->addWidget( welcomeLabel, 0, 1 );
    }
    if (kdmcfg->_showUsers != SHOW_NONE) {
	userView = new UserListView( winFrame );
	insertUsers( userView );
	main_grid->addMultiCellWidget(userView, 0, 3, 0, 0);
	connect( userView, SIGNAL(clicked( QListViewItem * )),
		 SLOT(slotUserClicked( QListViewItem * )) );
	connect( userView, SIGNAL(doubleClicked( QListViewItem * )),
		 SLOT(accept()) );
    }

    switch (kdmcfg->_logoArea) {
	case LOGO_CLOCK:
	    clock = new KdmClock( winFrame, "clock" );
	    break;
	case LOGO_LOGO:
	    {
		QPixmap pixmap;
		if (pixmap.load( kdmcfg->_logo )) {
		    pixLabel = new QLabel( winFrame );
		    pixLabel->setSizePolicy( QSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed ) );
		    pixLabel->setFrameStyle( QFrame::Panel | QFrame::Sunken );
		    pixLabel->setAutoResize( true );
		    pixLabel->setIndent( 0 );
		    pixLabel->setPixmap( pixmap );
		}
	    }
	    break;
    }

    main_grid->addItem( hbox1, 1, 1 );
    main_grid->addItem( grid, 2, 1 );
    main_grid->addItem( hbox2, 3, 1 );
    if (kdmcfg->_showUsers != SHOW_NONE) {
	if (clock)
	    hbox1->addWidget( clock, 0, AlignTop );
	else if (pixLabel)
	    hbox1->addWidget( pixLabel, 0, AlignTop );
    } else {
#if 0
	if (clock)
	    main_grid->addMultiCellWidget( clock, 0,3, 2,2, AlignCenter );
	else if (pixLabel)
	    main_grid->addMultiCellWidget( pixLabel, 0,3, 2,2, AlignCenter );
#else
	if (clock)
	    grid->addMultiCellWidget( clock, 0,3, 4,4, AlignTop );
	else if (pixLabel)
	    grid->addMultiCellWidget( pixLabel, 0,3, 4,4, AlignTop );
#endif
    }

    KSeparator* sep = new KSeparator( KSeparator::HLine, winFrame );
#if 0
    grid->addMultiCellWidget( sep, 4,4, 0,3 );
#else
    grid->addMultiCellWidget( sep, 4,4, 0,
	(kdmcfg->_showUsers != SHOW_NONE || 
	 kdmcfg->_logoArea == LOGO_NONE) ? 3 : 4 );
#endif
    grid->setColStretch( 3, 1 );

    goButton = new QPushButton( i18n("L&ogin"), winFrame );
    goButton->setFixedWidth( goButton->sizeHint().width() );
    goButton->setDefault( true );
    connect( goButton, SIGNAL( clicked()), SLOT(accept()) );
    hbox2->addWidget( goButton );

    clearButton = new QPushButton( i18n("&Clear"), winFrame );
    connect( clearButton, SIGNAL(clicked()), SLOT(reject()) );
    hbox2->addWidget( clearButton );

    hbox2->addStretch( 1 );

    QPushButton *menuButton = new QPushButton( i18n("&Menu"), winFrame );

    hbox2->addWidget( menuButton );

    hbox2->addStretch( 1 );

//helpButton

    QWidget *prec;
    if (userView)
	prec = userView;
#ifdef WITH_KDM_XCONSOLE // XXX won't work when moved to kgdialog
    else if (consoleView)
	prec = consoleView;
#endif
    else
	prec = menuButton;
    if (curPlugin < 0) {
	curPlugin = 0;
	pluginList = KGVerify::init( kdmcfg->_pluginsLogin );
    }
    verify = new KGVerify( this, winFrame, prec, QString::null,
			   pluginList, KGreeterPlugin::Authenticate,
			   KGreeterPlugin::Login );
    grid->addMultiCellLayout( verify->getLayout(), 0,3, 0,3, AlignCenter );
    verify->selectPlugin( curPlugin );

    sessMenu = new QPopupMenu( winFrame );
    connect( sessMenu, SIGNAL(activated(int)),
	     SLOT(slotSessionSelected(int)) );
    insertSessions();

    Inserten( i18n("Session &Type"), ALT+Key_T, sessMenu );

    optMenu->insertSeparator();

    QPopupMenu *plugMenu = verify->getPlugMenu();
    if (plugMenu) {
	Inserten( i18n("&Authentication Method"), ALT+Key_A, plugMenu );
	optMenu->insertSeparator();
    }

    completeMenu( LOGIN_LOCAL_ONLY, ex_choose, i18n("&Remote Login"), ALT+Key_R );

    menuButton->setPopup( optMenu );

#ifdef WITH_KDM_XCONSOLE
    move to kgdialog
    if (kdmcfg->_showLog) {
	consoleView = new KConsole( this, kdmcfg->_logSource );
	main_grid->addMultiCellWidget( consoleView, 4,4, 0,1 );
    }
#endif

    pluginSetup();

    verify->start();
}

KGreeter::~KGreeter()
{
    hide();
    delete verify;
    delete stsFile;
}

class UserListViewItem : public KListViewItem {
public:
    UserListViewItem( UserListView *parent, const QString &text,
		      const QPixmap &pixmap, const QString &username )
	: KListViewItem( parent )
	, login( username )
    {
	setPixmap( 0, pixmap );
	setMultiLinesEnabled( true );
	setText( 0, text );
	parent->cachedSizeHint.setWidth( -1 );
    }

    QString login;
};

void
KGreeter::insertUser( UserListView *listview, const QImage &default_pix,
		      const QString &username, struct passwd *ps )
{
    int dp = 0, nd = 0;
    if (kdmcfg->_faceSource == FACE_USER_ONLY ||
	kdmcfg->_faceSource == FACE_PREFER_USER)
	dp = 1;
    if (kdmcfg->_faceSource != FACE_USER_ONLY &&
	kdmcfg->_faceSource != FACE_ADMIN_ONLY)
	nd = 1;
    QImage p;
    // XXX remove seteuid-voodoo when we run as nobody
    seteuid( ps->pw_uid );
    do {
	QString fn = dp ?
		     QFile::decodeName( ps->pw_dir ) + "/.face" :
		     kdmcfg->_faceDir + '/' + username + ".face";
	if (p.load( fn + ".icon" ) || p.load( fn )) {
	    QSize ns( 48, 48 );
	    if (p.size() != ns)
		p = p.convertDepth( 32 ).smoothScale( ns, QImage::ScaleMin );
	    goto gotit;
	}
	dp = 1 - dp;
    } while (--nd >= 0);
    p = default_pix;
  gotit:
    seteuid( 0 );
    QString realname = QFile::decodeName( ps->pw_gecos );
    realname.truncate( realname.find( ',' ) );
    if (realname.isEmpty() || realname == username)
	new UserListViewItem( listview, username, QPixmap( p ), username );
    else {
	realname.append( "\n" ).append( username );
	new UserListViewItem( listview, realname, QPixmap( p ), username );
    }
}

void
KGreeter::insertUsers( UserListView *listview )
{
    QImage default_pix;
    if (!default_pix.load( kdmcfg->_faceDir + "/.default.face.icon" ))
	if (!default_pix.load( kdmcfg->_faceDir + "/.default.face" ))
	    LogError("Can't open default user face\n");
    QSize ns( 48, 48 );
    if (default_pix.size() != ns)
	default_pix =
	  default_pix.convertDepth( 32 ).smoothScale( ns, QImage::ScaleMin );
    struct passwd *ps;
    if (kdmcfg->_showUsers == SHOW_ALL) {
	QDict<int> users( 1000 );
	for (setpwent(); (ps = getpwent()) != 0;) {
	    // usernames are stored in the same encoding as files
	    QString username = QFile::decodeName( ps->pw_name );
	    if (*ps->pw_dir && *ps->pw_shell &&
		(ps->pw_uid >= (unsigned)kdmcfg->_lowUserId ||
		 !ps->pw_uid && kdmcfg->_showRoot) &&
		ps->pw_uid <= (unsigned)kdmcfg->_highUserId &&
		!kdmcfg->_noUsers.contains( username ) &&
		!users.find( username )
	    ) {
		// we might have a real user, insert him/her
		users.insert( username, (int *)-1 );
		insertUser( listview, default_pix, username, ps );
	    }
	}
    } else {
	QStringList::ConstIterator it = kdmcfg->_users.begin();
	for (; it != kdmcfg->_users.end(); ++it)
	    if ((ps = getpwnam( (*it).latin1() )))
		insertUser( listview, default_pix, *it, ps );
    }
    endpwent();
    if (kdmcfg->_sortUsers)
	listview->sort();
}

void
KGreeter::putSession(const QString &type, const QString &name, bool hid, const char *exe)
{
    int prio = exe ? (!strcmp( exe, "default" ) ? 0 :
			!strcmp( exe, "custom" ) ? 1 :
			  !strcmp( exe, "failsafe" ) ? 3 : 2) : 2;
    for (uint i = 0; i < sessionTypes.size(); i++)
	if (sessionTypes[i].type == type) {
	    sessionTypes[i].prio = prio;
	    return;
	}
    sessionTypes.append( SessType( name, type, hid, prio ) );
}

void
KGreeter::insertSessions()
{
    for (QStringList::ConstIterator dit = kdmcfg->_sessionsDirs.begin();
	 dit != kdmcfg->_sessionsDirs.end(); ++dit)
    {
	QStringList ents = QDir( *dit ).entryList();
	for (QStringList::ConstIterator it = ents.begin(); it != ents.end(); ++it)
	    if ((*it).endsWith( ".desktop" )) {
		KSimpleConfig dsk( QString( *dit ).append( '/' ).append( *it ) );
		dsk.setGroup( "Desktop Entry" );
		putSession( (*it).left( (*it).length() - 8 ),
			    dsk.readEntry( "Name" ),
			    (dsk.readBoolEntry( "Hidden", false ) ||
			     (dsk.hasKey( "TryExec" ) &&
			      KStandardDirs::findExe( dsk.readEntry( "TryExec" ) ).isEmpty())),
			    dsk.readEntry( "Exec" ).latin1() );
	    }
    }
    putSession( "default", i18n("Default"), false, "default" );
    putSession( "custom", i18n("Custom"), false, "custom" );
    putSession( "failsafe", i18n("Failsafe"), false, "failsafe" );
    qBubbleSort( sessionTypes );
    for (uint i = 0; i < sessionTypes.size() && !sessionTypes[i].hid; i++) {
	sessMenu->insertItem( sessionTypes[i].name, i );
	switch (sessionTypes[i].prio) {
	case 0: case 1: nSpecials++; break;
	case 2: nNormals++; break;
	}
    }
}

void
KGreeter::slotShutdown()
{
    verify->suspend();
    inherited::slotShutdown();
    verify->resume();
}

void
KGreeter::slotUserEntered()
{
    if (userView) {
	QListViewItem *item;
	for (item = userView->firstChild(); item; item = item->nextSibling())
	    if (((UserListViewItem *)item)->login == curUser) {
		userView->setCurrentItem( item );
		userView->ensureItemVisible( item );
		goto oke;
	    }
	userView->clearSelection();
    }
  oke:
    QTimer::singleShot( 0, this, SLOT(slotLoadPrevWM()) );
}

void
KGreeter::slotUserClicked( QListViewItem *item )
{
    if (item) {
	curUser = ((UserListViewItem *)item)->login;
	verify->setUser( curUser );
	slotLoadPrevWM();
    }
}

void
KGreeter::slotSessionSelected( int id )
{
    if (id != curSel) {
	sessMenu->setItemChecked( curSel, false );
	sessMenu->setItemChecked( id, true );
	curSel = id;
    }
}

void
KGreeter::reject()
{
    verify->reject();
    curUser = QString::null;
    slotUserEntered();
    slotSessionSelected( -1 );
}

void
KGreeter::accept()
{
    if (userView && userView->hasFocus())
	slotUserClicked( userView->currentItem() );
    else
	verify->accept();
}

void // private
KGreeter::setPrevWM( int wm )
{
    if (curPrev != wm) {
	if (curPrev != -1)
	    sessMenu->changeItem( curPrev, sessionTypes[curPrev].name );
	if (wm != -1)
	    sessMenu->changeItem( wm, sessionTypes[wm].name + i18n(" (previous)") );
	curPrev = wm;
    }
}

void
KGreeter::slotLoadPrevWM()
{
    int len, i, b;
    unsigned long crc, by;
    QCString name;
    char *sess;

    if (verify->coreLock) {
	needLoad = true;
	return;
    }
    needLoad = false;

    prevValid = true;
    name = curUser.local8Bit();
    GSendInt( G_ReadDmrc );
    GSendStr( name.data() );
    GRecvInt(); // ignore status code ...
    if ((len = name.length())) {
	GSendInt( G_GetDmrc );
	GSendStr( "Session" );
	sess = GRecvStr();
	if (!sess) {		/* no such user */
	    if (kdmcfg->_showUsers == SHOW_NONE) { // don't fake if user list shown
		/* simple crc32 */
		for (crc = kdmcfg->_forgingSeed, i = 0; i < len; i++) {
		    by = (crc & 255) ^ name[i];
		    for (b = 0; b < 8; b++)
			by = (by >> 1) ^ (-(by & 1) & 0xedb88320);
		    crc = (crc >> 8) ^ by;
		}
		/* forge a session with this hash - default & custom more probable */
		/* XXX - this should do a statistical analysis of the real users */
#if 1
		setPrevWM( crc % (nSpecials * 2 + nNormals) % (nSpecials + nNormals) );
#else
		i = crc % (nSpecials * 2 + nNormals);
		if (i < nNormals)
		    setPrevWM( i + nSpecials );
		else
		    setPrevWM( (i - nNormals) / 2 );
#endif
		return;
	    }
	} else {
	    for (uint i = 0; i < sessionTypes.count(); i++)
		if (sessionTypes[i].type == sess) {
		    free( sess );
		    setPrevWM( i );
		    return;
		}
	    if (curSel == -1)
		MsgBox( sorrybox, i18n("Your saved session type '%1' is not valid any more.\n"
		    "Please select a new one, otherwise 'default' will be used.").arg(sess) );
	    free( sess );
	    prevValid = false;
	}
    }
    setPrevWM( -1 );
}

void // private
KGreeter::pluginSetup()
{
    if (kdmcfg->_preselUser != PRESEL_PREV)
	stsFile->deleteEntry( dName, false );
    if (kdmcfg->_preselUser != PRESEL_NONE)
	verify->presetEntity(
		kdmcfg->_preselUser == PRESEL_PREV ?
		    stsFile->readEntry( dName ) : kdmcfg->_defaultUser,
		kdmcfg->_focusPasswd );
    if (userView) {
	if (verify->isPluginLocal())
	    userView->show();
	else
	    userView->hide();
    }
    adjustGeometry();
}

void
KGreeter::verifyPluginChanged( int id )
{
    curPlugin = id;
    pluginSetup();
}

void
KGreeter::verifyOk()
{
    if (kdmcfg->_preselUser == PRESEL_PREV)
	stsFile->writeEntry( dName, verify->getEntity() );
    hide();
    if (curSel != -1) {
	GSendInt( G_PutDmrc );
	GSendStr( "Session" );
	GSendStr( sessionTypes[curSel].type.utf8() );
    } else if (!prevValid) {
	GSendInt( G_PutDmrc );
	GSendStr( "Session" );
	GSendStr( "default" );
    }
    GSendInt( G_Ready );
    done( ex_exit );
}

void
KGreeter::verifyFailed()
{
    goButton->setEnabled( false );
    if (needLoad)
	slotLoadPrevWM();
}

void
KGreeter::verifyRetry()
{
    goButton->setEnabled( true );
}

void
KGreeter::verifySetUser( const QString &user )
{
    curUser = user;
    slotUserEntered();
}

#include "kgreeter.moc"
