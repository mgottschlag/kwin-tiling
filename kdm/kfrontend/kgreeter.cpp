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
#include "kconsole.h"
#include "kdmconfig.h"
#include "kdmclock.h"
#include "kdm_greet.h"

#include <kapplication.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kseparator.h>
#include <klistview.h>
#include <ksimpleconfig.h>
#include <kstringhandler.h>

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
#include <qtooltip.h>

#include <pwd.h>
#include <grp.h>
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
  , userList( 0 )
  , clock( 0 )
  , pixLabel( 0 )
  , nNormals( 0 )
  , nSpecials( 0 )
  , curPrev( -1 )
  , curSel( -1 )
  , prevValid( true )
  , needLoad( false )
{
    stsFile = new KSimpleConfig( _stsFile );
    stsFile->setGroup( "PrevUser" );

#ifdef WITH_KDM_XCONSOLE
    QBoxLayout *main_box = new QHBoxLayout( 10 );
    layout->addLayout( main_box, 0, 0 );
#else
    QBoxLayout *main_box = new QHBoxLayout( this, 10, 10 );
#endif

    if (_userList) {
	userView = new UserListView( this );
	main_box->addWidget(userView);
	connect( userView, SIGNAL(clicked( QListViewItem * )),
		 SLOT(slotUserClicked( QListViewItem * )) );
	connect( userView, SIGNAL(doubleClicked( QListViewItem * )),
		 SLOT(accept()) );
    }

    QBoxLayout *inner_box = new QVBoxLayout( main_box, 10 );

    if (!_authorized && _authComplain) {
	QLabel* complainLabel = new QLabel(
	    i18n("Warning: this is an unsecured session"), this );
	QToolTip::add( complainLabel,
	    i18n("This display requires no X authorization.\n"
		 "This means that anybody can connect to it,\n"
		  "open windows on it or intercept your input.") );
	complainLabel->setAlignment( AlignCenter );
	complainLabel->setFont( _failFont );
	complainLabel->setPaletteForegroundColor( Qt::red );
	inner_box->addWidget( complainLabel );
    }
    if (!_greetString.isEmpty()) {
	QLabel* welcomeLabel = new QLabel( _greetString, this );
	welcomeLabel->setAlignment( AlignCenter );
	welcomeLabel->setFont( _greetFont );
	inner_box->addWidget( welcomeLabel );
    }
    if (_userCompletion)
	userList = new QStringList;
    if (userView || userList)
	insertUsers();

    switch (_logoArea) {
	case LOGO_CLOCK:
	    clock = new KdmClock( this, "clock" );
	    break;
	case LOGO_LOGO:
	    {
		QPixmap pixmap;
		if (pixmap.load( _logo )) {
		    pixLabel = new QLabel( this );
		    pixLabel->setSizePolicy( QSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed ) );
		    pixLabel->setFrameStyle( QFrame::Panel | QFrame::Sunken );
		    pixLabel->setAutoResize( true );
		    pixLabel->setIndent( 0 );
		    pixLabel->setPixmap( pixmap );
		}
	    }
	    break;
    }

    if (userView) {
	if (clock)
	    inner_box->addWidget( clock, 0, AlignCenter );
	else if (pixLabel)
	    inner_box->addWidget( pixLabel, 0, AlignCenter );
    } else {
	if (clock)
	    main_box->addWidget( clock, 0, AlignCenter );
	else if (pixLabel)
	    main_box->addWidget( pixLabel, 0, AlignCenter );
    }

    goButton = new QPushButton( i18n("L&ogin"), this );
    goButton->setDefault( true );
    connect( goButton, SIGNAL( clicked()), SLOT(accept()) );
    QPushButton *menuButton = new QPushButton( i18n("&Menu"), this );
    //helpButton

    QWidget *prec;
    if (userView)
	prec = userView;
#ifdef WITH_KDM_XCONSOLE
    else if (consoleView)
	prec = consoleView;
#endif
    else
	prec = menuButton;
    if (curPlugin < 0) {
	curPlugin = 0;
	pluginList = KGVerify::init( _pluginsLogin );
    }
    verify = new KGVerify( this, this, prec, QString::null,
			   pluginList, KGreeterPlugin::Authenticate,
			   KGreeterPlugin::Login );
    inner_box->addLayout( verify->getLayout() );
    verify->selectPlugin( curPlugin );

    inner_box->addWidget( new KSeparator( KSeparator::HLine, this ) );

    QBoxLayout* hbox2 = new QHBoxLayout( inner_box, 10 );
    hbox2->addWidget( goButton );
    hbox2->addStretch( 1 );
    hbox2->addWidget( menuButton );
    hbox2->addStretch( 1 );

    sessMenu = new QPopupMenu( this );
    connect( sessMenu, SIGNAL(activated(int)),
	     SLOT(slotSessionSelected(int)) );
    insertSessions();

    if (sessMenu->count() > 1) {
	inserten( i18n("Session &Type"), ALT+Key_T, sessMenu );
	needSep = true;
    }

    QPopupMenu *plugMenu = verify->getPlugMenu();
    if (plugMenu) {
	inserten( i18n("&Authentication Method"), ALT+Key_A, plugMenu );
	needSep = true;
    }

#ifdef XDMCP
    completeMenu( LOGIN_LOCAL_ONLY, ex_choose, i18n("&Remote Login"), ALT+Key_R );
#else
    completeMenu();
#endif

    if (optMenu)
	menuButton->setPopup( optMenu );
    else
	menuButton->hide();

    pluginSetup();

    verify->start();
}

KGreeter::~KGreeter()
{
    hide();
    delete userList;
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
KGreeter::insertUser( const QImage &default_pix,
		      const QString &username, struct passwd *ps )
{
    if (userList)
	userList->append( username );
    if (!userView)
	return;

    int dp = 0, nd = 0;
    if (_faceSource == FACE_USER_ONLY ||
	_faceSource == FACE_PREFER_USER)
	dp = 1;
    if (_faceSource != FACE_USER_ONLY &&
	_faceSource != FACE_ADMIN_ONLY)
	nd = 1;
    QImage p;
    // XXX remove seteuid-voodoo when we run as nobody
    seteuid( ps->pw_uid );
    do {
	QString fn = dp ?
		     QFile::decodeName( ps->pw_dir ) + "/.face" :
		     _faceDir + '/' + username + ".face";
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
    QString realname = KStringHandler::from8Bit( ps->pw_gecos );
    realname.truncate( realname.find( ',' ) );
    if (realname.isEmpty() || realname == username)
	new UserListViewItem( userView, username, QPixmap( p ), username );
    else {
	realname.append( "\n" ).append( username );
	new UserListViewItem( userView, realname, QPixmap( p ), username );
    }
}

class KCStringList : public QValueList<QCString> {
public:
    bool contains( const char *str ) const
    {
	for (ConstIterator it = begin(); it != end(); ++it)
	    if (*it == str)
		return true;
	return false;
    }
};

class UserList {
public:
    UserList( char **in );
    bool hasUser( const char *str ) const { return users.contains( str ); }
    bool hasGroup( gid_t gid ) const
	{ return groups.find( gid ) != groups.end(); }
    bool hasGroups() const { return !groups.isEmpty(); }
    KCStringList users;
private:
    QValueList<gid_t> groups;
};

UserList::UserList( char **in )
{
    struct group *grp;

    for (; *in; in++)
	if (**in == '@') {
	    if ((grp = getgrnam( *in + 1 ))) {
		for (; *grp->gr_mem; grp->gr_mem++)
		    users.append( *grp->gr_mem );
		groups.append( grp->gr_gid );
	    }
	} else
	    users.append( *in );
}

void
KGreeter::insertUsers()
{
    QImage default_pix;
    if (userView) {
	if (!default_pix.load( _faceDir + "/.default.face.icon" ))
	    if (!default_pix.load( _faceDir + "/.default.face" ))
		LogError("Can't open default user face\n");
	QSize ns( 48, 48 );
	if (default_pix.size() != ns)
	    default_pix =
	      default_pix.convertDepth( 32 ).smoothScale( ns, QImage::ScaleMin );
    }
    struct passwd *ps;
    if (_showUsers == SHOW_ALL) {
	UserList noUsers( _noUsers );
	QDict<int> dupes( 1000 );
	for (setpwent(); (ps = getpwent()) != 0;) {
	    if (*ps->pw_dir && *ps->pw_shell &&
		(ps->pw_uid >= (unsigned)_lowUserId ||
		 !ps->pw_uid && _showRoot) &&
		ps->pw_uid <= (unsigned)_highUserId &&
		!noUsers.hasUser( ps->pw_name ) &&
		!noUsers.hasGroup( ps->pw_gid )
	    ) {
		QString username( QFile::decodeName( ps->pw_name ) );
		if (!dupes.find( username )) {
		    dupes.insert( username, (int *)-1 );
		    insertUser( default_pix, username, ps );
		}
	    }
	}
    } else {
	UserList users( _users );
	if (users.hasGroups()) {
	    QDict<int> dupes( 1000 );
	    for (setpwent(); (ps = getpwent()) != 0;) {
		if (*ps->pw_dir && *ps->pw_shell &&
		    (ps->pw_uid >= (unsigned)_lowUserId ||
		     !ps->pw_uid && _showRoot) &&
		    ps->pw_uid <= (unsigned)_highUserId &&
		    (users.hasUser( ps->pw_name ) ||
		     users.hasGroup( ps->pw_gid ))
		) {
		    QString username( QFile::decodeName( ps->pw_name ) );
		    if (!dupes.find( username )) {
			dupes.insert( username, (int *)-1 );
			insertUser( default_pix, username, ps );
		    }
		}
	    }
	} else {
	    KCStringList::ConstIterator it = users.users.begin();
	    for (; it != users.users.end(); ++it)
		if ((ps = getpwnam( (*it).data() )))
		    insertUser( default_pix, QFile::decodeName( *it ), ps );
	}
    }
    endpwent();
    if (_sortUsers) {
	if (userView)
	    userView->sort();
	if (userList)
	    userList->sort();
    }
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
    for (char **dit = _sessionsDirs; *dit; ++dit) {
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
KGreeter::slotConsole()
{
    verify->abort();
    inherited::slotConsole();
}

void
KGreeter::slotExit()
{
    verify->abort();
    inherited::slotExit();
}

void
KGreeter::slotUserEntered()
{
    if (userView) {
	QListViewItem *item;
	for (item = userView->firstChild(); item; item = item->nextSibling())
	    if (((UserListViewItem *)item)->login == curUser) {
		userView->setSelected( item, true );
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
	    if (!userView && !userList) { // don't fake if user list shown
		prevValid = false;
		/* simple crc32 */
		for (crc = _forgingSeed, i = 0; i < len; i++) {
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
    int field = 0;
    QString ent, pn( verify->pluginName() ), dn( dName + '_' + pn );

    if (_preselUser != PRESEL_PREV)
	stsFile->deleteEntry( verify->entitiesLocal() ? dName : dn, false );
    if (_preselUser != PRESEL_NONE) {
	if (verify->entitiesLocal())
	    ent = _preselUser == PRESEL_PREV ?
		stsFile->readEntry( dName ) : _defaultUser;
	else
	    ent = _preselUser == PRESEL_PREV ?
		stsFile->readEntry( dn ) :
		verify->getConf( 0, (pn + ".DefaultEntity").latin1(), QVariant() ).toString();
	field = verify->entitiesFielded() ?
	    verify->getConf( 0, (pn + ".FocusField").latin1(), QVariant( 0 ) ).toInt() :
	   _focusPasswd;
    }
    if (!ent.isEmpty())
	verify->presetEntity( ent, field );
    if (userView) {
	if (verify->entitiesLocal())
	    userView->show();
	else
	    userView->hide();
    }
    if (userList)
	verify->loadUsers( *userList );
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
    if (_preselUser == PRESEL_PREV)
	stsFile->writeEntry(
		verify->entitiesLocal() ?
			dName :
			dName + '_' + verify->pluginName(),
		verify->getEntity() );
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
