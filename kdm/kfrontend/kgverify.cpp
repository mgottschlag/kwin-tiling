    /*

    Shell for kdm conversation plugins

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

#include <config.h>

#include "kgverify.h"
#include "kdmconfig.h"
#include "kdm_greet.h"

#include <kapplication.h>
#include <klocale.h>
#include <klibloader.h>
#include <kseparator.h>
#include <kstdguiitem.h>
#include <kpushbutton.h>

#include <qpopupmenu.h>
#include <qlayout.h>
#include <qfile.h>
#include <qlabel.h>

#include <pwd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>

#include <X11/Xlib.h> // for updateLockStatus()
#include <fixx11h.h> // ... and make eventFilter() work again


KGVerify::KGVerify( KGVerifyHandler *_handler, QWidget *_parent,
		    QWidget *_predecessor, const QString &_fixedUser,
		    const PluginList &_pluginList,
		    KGreeterPlugin::Function _func,
		    KGreeterPlugin::Context _ctx )
  : inherited()
  , coreLock( 0 )
  , fixedEntity( _fixedUser )
  , pluginList( _pluginList )
  , handler( _handler )
  , parent( _parent )
  , predecessor( _predecessor )
  , plugMenu( 0 )
  , curPlugin( -1 )
  , failedLabelState( -1 )
  , func( _func )
  , ctx( _ctx )
  , enabled( true )
  , running( false )
  , suspended( false )
  , failed( false )
{
    grid = new QGridLayout;

    failedLabel = new QLabel( parent );
    failedLabel->setFont( kdmcfg->_failFont );
    grid->addWidget( failedLabel, 1, 0, AlignCenter );

    connect( &timer, SIGNAL(timeout()), SLOT(slotTimeout()) );

    _parent->installEventFilter( this );
    updateLockStatus();
}

KGVerify::~KGVerify()
{
    grid->removeItem( greet->getLayoutItem() );
    Debug( "delete greet\n" );
    delete greet;
}

QPopupMenu *
KGVerify::getPlugMenu()
{
    // assert( !cont );
    if (!plugMenu) {
	uint np = pluginList.count();
	if (np > 1) {
	    plugMenu = new QPopupMenu( parent );
	    connect( plugMenu, SIGNAL(activated(int)),
		     SLOT(slotPluginSelected(int)) );
	    for (uint i = 0; i < np; i++)
		plugMenu->insertItem( i18n( greetPlugins[pluginList[i]].info->name ), pluginList[i] );
	}
    }
    return plugMenu;
}

bool // public
KGVerify::isPluginLocal() const
{
    return greetPlugins[pluginList[curPlugin]].info->flags & kgreeterplugin_info::Local;
}

QString // public
KGVerify::pluginName() const
{
    QString name( greetPlugins[pluginList[curPlugin]].library->fileName() );
    uint st = name.findRev( '/' ) + 1;
    uint en = name.find( '.', st );
    if (en - st > 7 && QConstString( name.unicode() + st, 7 ).string() == "kgreet_" )
	st += 7;
    return name.mid( st, en - st );
}

void // public
KGVerify::selectPlugin( int id )
{
    if (pluginList.isEmpty()) {
	MsgBox( errorbox, i18n("No greeter widget plugin loaded. Check the configuration.") );
	::exit( EX_UNMANAGE_DPY );
    }
    curPlugin = id;
    greet = greetPlugins[pluginList[id]].info->create( this, parent, predecessor, fixedEntity, func, ctx );
    grid->addItem( greet->getLayoutItem(), 0, 0 );
    if (plugMenu)
	plugMenu->setItemChecked( id, true );
}

void // private slot
KGVerify::slotPluginSelected( int id )
{
    if (failed)
	return;
    if (id != curPlugin) {
	plugMenu->setItemChecked( curPlugin, false );
	parent->setUpdatesEnabled( false );
	grid->removeItem( greet->getLayoutItem() );
	Debug( "delete greet\n" );
	delete greet;
	selectPlugin( id );
	handler->verifyPluginChanged( id );
	if (running)
	    start();
	parent->setUpdatesEnabled( true );
    }
}

void // public
KGVerify::loadUsers( const QStringList &users )
{
    Debug( "greet->loadUsers(...)\n" );
    greet->loadUsers( users );
}

void // public
KGVerify::presetEntity( const QString &entity, int field )
{
    Debug( "greet->presetEntity(%\"s, %d)\n", entity.latin1(), field );
    greet->presetEntity( entity, field );
    timer.stop();
}

QString // public
KGVerify::getEntity() const
{
    Debug( "greet->getEntity()\n" );
    return greet->getEntity();
}

void
KGVerify::setUser( const QString &user )
{
    // assert( fixedEntity.isEmpty() );
    curUser = user;
    Debug( "greet->setUser(%\"s)\n", user.latin1() );
    greet->setUser( user );
    hasBegun = true;
    setTimer();
}

void
KGVerify::start()
{
    hasBegun = false;
    authTok = (func == KGreeterPlugin::ChAuthTok);
    reAuthTok = false;
    running = true;
    Debug( "greet->start()\n" );
    greet->start();
    cont = false;
    if (!(func == KGreeterPlugin::Authenticate ||
	  ctx == KGreeterPlugin::ChangeTok ||
	  ctx == KGreeterPlugin::ExChangeTok))
    {
	cont = true;
	handleVerify();
    }
}

void
KGVerify::abort()
{
    Debug( "greet->abort()\n" );
    greet->abort();
    running = false;
}

void
KGVerify::suspend()
{
    // assert( !cont );
    if (running) {
	Debug( "greet->abort()\n" );
	greet->abort();
    }
    suspended = true;
    updateStatus();
}

void
KGVerify::resume()
{
    suspended = false;
    updateLockStatus();
    if (running) {
	Debug( "greet->start()\n" );
	greet->start();
    } else if (delayed) {
	delayed = false;
	running = true;
	Debug( "greet->start()\n" );
	greet->start();
	setTimer();
    }
}

void
KGVerify::accept()
{
    Debug( "greet->next()\n" );
    greet->next();
}

void
KGVerify::reject()
{
    // assert( !cont );
    curUser = QString::null;
    if (running) {
	Debug( "greet->abort()\n" );
	greet->abort();
    }
    Debug( "greet->clear()\n" );
    greet->clear();
    if (running) {
	Debug( "greet->start()\n" );
	greet->start();
    }
    hasBegun = false;
    if (!failed)
	timer.stop();
}

void
KGVerify::setEnabled( bool on )
{
    Debug( "greet->setEnabled(%s)\n", on ? "true" : "false" );
    greet->setEnabled( on );
    enabled = on;
    updateStatus();
}

void // private
KGVerify::slotTimeout()
{
    if (failed) {
	failed = false;
	updateStatus();
	Debug( "greet->revive()\n" );
	greet->revive();
	handler->verifyRetry();
	if (suspended)
	    delayed = true;
	else {
	    running = true;
	    Debug( "greet->start()\n" );
	    greet->start();
	    setTimer();
	    if (cont)
		handleVerify();
	}
    } else {
	// assert( ctx == Login );
	reject();
	handler->verifySetUser( QString::null );
    }
}

void // private
KGVerify::setTimer()
{
    if (!failed && fixedEntity.isEmpty() && hasBegun)
	timer.start( 40000 );
}


void // private static
KGVerify::VMsgBox( QWidget *parent, const QString &user,
		   QMessageBox::Icon type, const QString &mesg )
{
    FDialog::box( parent, type, user.isEmpty() ?
	mesg : i18n("Authenticating %1 ...\n\n").arg(user) + mesg );
}

bool // private static
KGVerify::handleNTVerify( QWidget *parent, int ret, const QString &user )
{
    char *msg;
    int expire;
    QString mesg;
    QMessageBox::Icon icon;

    switch (ret) {
    case V_MSG_ERR:
	Debug( " V_MSG_ERR\n" );
	msg = GRecvStr();
	Debug( "  message %\"s\n", msg );
	mesg = QString::fromLocal8Bit( msg );
	free( msg );
	icon = sorrybox;
	break;
    case V_MSG_INFO:
	Debug( " V_MSG_INFO\n" );
	msg = GRecvStr();
	Debug( "  message %\"s\n", msg );
	mesg = QString::fromLocal8Bit( msg );
	free( msg );
	icon = infobox;
	break;
    case V_PEXPIRED:
	Debug( " V_PEXPIRED\n" );
	mesg = i18n("You are required to change your password immediately (password aged).");
	icon = sorrybox;
	break;
    case V_PFEXPIRED:
	Debug( " V_PFEXPIRED\n" );
	mesg = i18n("You are required to change your password immediately (root enforced).");
	icon = sorrybox;
	break;
    case V_AWEXPIRE:
	Debug( " V_AWEXPIRE\n" );
	expire = GRecvInt();
	Debug( "  in %d days\n", expire );
	mesg = expire ?
	    i18n("Your account expires tomorrow.", "Your account expires in %n days.", expire) :
	    i18n("Your account expires today.");
	icon = infobox;
	break;
    case V_PWEXPIRE:
	Debug( " V_PWEXPIRE\n" );
	expire = GRecvInt();
	Debug( "  in %d days\n", expire );
	mesg = expire ?
	    i18n("Your password expires tomorrow.", "Your password expires in %n days.", expire) :
	    i18n("Your password expires today.");
	icon = infobox;
	break;
    default:
	return false;
    }
    VMsgBox( parent, user, icon, mesg );
    return true;
}

bool // private static
KGVerify::handleTVerify( QWidget *parent, int ret, const QString &user )
{
    char *msg;
    QString mesg;
    QMessageBox::Icon icon;

    switch (ret) {
    case V_ERROR:
	Debug( " V_ERROR\n" );
	mesg = i18n("A critical error occurred.\n"
		    "Please look at KDM's logfile(s) for more information\n"
		    "or contact your system administrator.");
	icon = errorbox;
	break;
    case V_NOHOME:
	Debug( " V_NOHOME\n" );
	mesg = i18n("Home folder not available.");
	icon = sorrybox;
	break;
    case V_NOROOT:
	Debug( " V_NOROOT\n" );
	mesg = i18n("Root logins are not allowed.");
	icon = sorrybox;
	break;
    case V_BADSHELL:
	Debug( " V_BADSHELL\n" );
	mesg = i18n("Your login shell is not listed in /etc/shells.");
	icon = sorrybox;
	break;
    case V_AEXPIRED:
	Debug( " V_AEXPIRED\n" );
	mesg = i18n("Your account has expired.");
	icon = sorrybox;
	break;
    case V_APEXPIRED:
	Debug( " V_APEXPIRED\n" );
	mesg = i18n("Your account has expired (failed to change password).");
	icon = sorrybox;
	break;
    case V_BADTIME:
	Debug( " V_BADTIME\n" );
	mesg = i18n("You are not allowed to login at the moment.");
	icon = sorrybox;
	break;
    case V_NOLOGIN:
	Debug( " V_NOLOGIN\n" );
	msg = GRecvStr();
	Debug( "  file %\"s\n", msg );
	mesg = QString::fromLocal8Bit( msg );
	free( msg );
	{
	    QFile f( mesg );
	    f.open( IO_ReadOnly );
	    QByteArray tx( f.readAll() );
	    f.close();
	    mesg = QString::fromLocal8Bit( tx.data(), tx.size() );
	}
	if (mesg.isEmpty())
	    mesg = i18n("Logins are not allowed at the moment.\n"
			"Try again later.");
	icon = sorrybox;
	break;
    default:
	return false;
    }
    VMsgBox( parent, user, icon, mesg );
    return true;
}

bool // public static
KGVerify::handleFailVerify( QWidget *parent )
{
    Debug( "handleFailVerify ...\n" );
    char *msg = GRecvStr();
    QString user = QString::fromLocal8Bit( msg );
    free( msg );

    for (;;) {
	int ret = GRecvInt();

	// non-terminal status
	switch (ret) {
	/* case V_PUT_USER: cannot happen - we are in "classic" mode */
	/* case V_PRE_OK: cannot happen - not in ChTok dialog */
	/* case V_CHTOK: cannot happen - called by non-interactive verify */
	case V_CHTOK_AUTH:
	    Debug( " V_CHTOK_AUTH\n" );
	    {
		QStringList pgs( kdmcfg->_pluginsLogin );
		pgs += kdmcfg->_pluginsShutdown;
		QStringList::ConstIterator it;
		for (it = pgs.begin(); it != pgs.end(); ++it)
		    if (*it == "classic" || *it == "modern") {
			pgs = *it;
			goto gotit;
		    } else if (*it == "generic") {
			pgs = "modern";
			goto gotit;
		    }
		pgs = "classic";
	      gotit:
		KGChTok chtok( parent, user, init( pgs ), 0,
			       KGreeterPlugin::AuthChAuthTok,
			       KGreeterPlugin::Login );
		return chtok.exec();
	    }
	}
	if (handleNTVerify( parent, ret, user ))
	    continue;

	// terminal status
	switch (ret) {
	case V_OK:
	    Debug( " V_OK\n" );
	    return true;
	case V_AUTH:
	    Debug( " V_AUTH\n" );
	    VMsgBox( parent, user, sorrybox, i18n("Authentication failed") );
	    return false;
	}
	if (!handleTVerify( parent, ret, user ))
	    LogPanic( "Unknown V_xxx code %d from core\n", ret );
	return false;
    }
}

void // private
KGVerify::handleVerify()
{
    QString user;

    Debug( "handleVerify ...\n" );
    for (;;) {
	char *msg;
	int ret, echo, ndelay;
	KGreeterPlugin::Function nfunc;

	ret = GRecvInt();

	// requests
	coreLock = 1;
	switch (ret) {
	case V_GET_TEXT:
	    Debug( " V_GET_TEXT\n" );
	    msg = GRecvStr();
	    Debug( "  prompt %\"s\n", msg );
	    echo = GRecvInt();
	    Debug( "  echo = %d\n", echo );
	    ndelay = GRecvInt();
	    Debug( "  ndelay = %d\n", ndelay );
	    if (reAuthTok) {
		reAuthTok = false;
		Debug( "greet->start()\n" );
		greet->start();
	    }
	    Debug( "greet->textPrompt(...)\n" );
	    greet->textPrompt( msg, echo, ndelay );
	    if (msg)
		free( msg );
	    return;
	case V_GET_BINARY:
	    Debug( " V_GET_BINARY\n" );
	    msg = GRecvArr( &ret );
	    Debug( "  %d bytes prompt\n", ret );
	    ndelay = GRecvInt();
	    Debug( "  ndelay = %d\n", ndelay );
	    if (reAuthTok) {
		reAuthTok = false;
		Debug( "greet->start()\n" );
		greet->start();
	    }
	    Debug( "greet->binaryPrompt(...)\n" );
	    greet->binaryPrompt( msg, ndelay );
	    if (msg)
		free( msg );
	    return;
	}

	// non-terminal status
	coreLock = 2;
	switch (ret) {
	case V_PUT_USER:
	    Debug( " V_PUT_USER\n" );
	    msg = GRecvStr();
	    Debug( "  user %\"s\n", msg );
	    curUser = user = QString::fromLocal8Bit( msg );
	    // greet needs this to be able to return something useful from
	    // getEntity(). but the backend is still unable to tell a domain ...
	    Debug( "  greet->setUser()\n" );
	    greet->setUser( curUser );
	    handler->verifySetUser( curUser );
	    if (msg)
		free( msg );
	    continue;
	case V_PRE_OK: // this is only for func == AuthChAuthTok
	    Debug( " V_PRE_OK\n" );
	    // With the "classic" method, the wrong user simply cannot be
	    // authenticated, even with the generic plugin. Other methods
	    // could do so, but this applies only to ctx == ChangeTok, which
	    // is not implemented yet.
	    authTok = true;
	    cont = true;
	    Debug( "greet->succeeded()\n" );
	    greet->succeeded();
	    continue;
	case V_CHTOK_AUTH:
	    Debug( " V_CHTOK_AUTH\n" );
	    nfunc = KGreeterPlugin::AuthChAuthTok;
	    user = curUser;
	    goto dchtok;
	case V_CHTOK:
	    Debug( " V_CHTOK\n" );
	    nfunc = KGreeterPlugin::ChAuthTok;
	    user = QString::null;
	  dchtok:
	    {
		timer.stop();
		Debug( "greet->succeeded()\n" );
		greet->succeeded();
		KGChTok chtok( parent, user, pluginList, curPlugin, nfunc, KGreeterPlugin::Login );
		if (!chtok.exec())
		    goto retry;
		handler->verifyOk();
		return;
	    }
	case V_MSG_ERR:
	    Debug( " V_MSG_ERR\n" );
	    msg = GRecvStr();
	    Debug( "  message %\"s\n", msg );
	    if (!greet->textMessage( msg, true ))
		VMsgBox( parent, user, sorrybox, QString::fromLocal8Bit( msg ) );
	    free( msg );
	    goto ntdone;
	case V_MSG_INFO:
	    Debug( " V_MSG_INFO\n" );
	    msg = GRecvStr();
	    Debug( "  message %\"s\n", msg );
	    if (!greet->textMessage( msg, false ))
		VMsgBox( parent, user, infobox, QString::fromLocal8Bit( msg ) );
	    free( msg );
	    goto ntdone;
	}
	if (handleNTVerify( parent, ret, user )) {
	  ntdone:
	    if (authTok)
		reAuthTok = true;
	    continue;
	}

	// terminal status
	coreLock = 0;
	running = false;
	timer.stop();

	if (ret == V_OK) {
	    Debug( " V_OK\n" );
	    if (!fixedEntity.isEmpty()) {
		QString ent = greet->getEntity();
		if (ent != fixedEntity) {
		    Debug( "greet->failed()\n" );
		    greet->failed();
		    MsgBox( sorrybox,
			i18n("Authenticated user (%1) does not match requested user (%2).\n")
			    .arg(ent).arg(fixedEntity) );
		    goto retry;
		}
	    }
	    Debug( "greet->succeeded()\n" );
	    greet->succeeded();
	    handler->verifyOk();
	    return;
	}

	Debug( "greet->failed()\n" );
	greet->failed();

	if (ret == V_RETRY) {
	    Debug( " V_RETRY\n" );
	    goto retry;
	}
	if (ret == V_AUTH) {
	    Debug( " V_AUTH\n" );
	    failed = true;
	    updateStatus();
	    handler->verifyFailed();
	    timer.start( 1500 + kapp->random()/(RAND_MAX/1000) );
	    return;
	}
	if (!handleTVerify( parent, ret, user ))
	    LogPanic( "Unknown V_xxx code %d from core\n", ret );
	curUser = QString::null;
	handler->verifySetUser( QString::null );
	Debug( "greet->clear()\n" );
	greet->clear();
      retry:
	Debug( "greet->revive()\n" );
	greet->revive();
	running = true;
	Debug( "greet->start()\n" );
	greet->start();
	reAuthTok = false;
	if (!cont)
	    return;
	user = QString::null;
    }
}

void
KGVerify::gplugReturnText( const char *text, int tag )
{
    Debug( "gplugReturnText(%\"s, %d)\n",
	    tag == V_IS_PASSWORD ? "<masked>" : text, tag );
    GSendStr( text );
    if (text) {
	GSendInt( tag );
	hasBegun = true;
	setTimer();
	handleVerify();
    } else
	coreLock = 0;
}

void
KGVerify::gplugReturnBinary( const char *data )
{
    if (data) {
	unsigned const char *up = (unsigned const char *)data;
	int len = up[3] | (up[2] << 8) | (up[1] << 16) | (up[0] << 24);
	Debug( "gplugReturnBinary(%d bytes)\n", len );
	GSendArr( len, data );
	hasBegun = true;
	setTimer();
	handleVerify();
    } else {
	Debug( "gplugReturnBinary(NULL)\n" );
	GSendArr( 0, 0 );
	coreLock = 0;
    }
}

void
KGVerify::gplugSetUser( const QString &user )
{
    Debug( "gplugSetUser(%\"s)\n", user.latin1() );
    curUser = user;
    handler->verifySetUser( user );
    hasBegun = !user.isEmpty();
    setTimer();
}

void
KGVerify::gplugStart()
{
    // XXX handle func != Authenticate
    Debug( "gplugStart()\n" );
    GSendInt( ctx == KGreeterPlugin::Shutdown ? G_VerifyRootOK : G_Verify );
    GSendStr( greetPlugins[pluginList[curPlugin]].info->method );
    handleVerify();
}

void
KGVerify::gplugActivity()
{
    setTimer();
}

bool
KGVerify::eventFilter( QObject *o, QEvent *e )
{
    switch (e->type()) {
    case QEvent::KeyPress:
    case QEvent::KeyRelease:
	updateLockStatus();
    default:
	break;
    }
    return inherited::eventFilter( o, e );
}

void
KGVerify::updateLockStatus()
{
    unsigned int lmask;
    Window dummy1, dummy2;
    int dummy3, dummy4, dummy5, dummy6;
    XQueryPointer( qt_xdisplay(), DefaultRootWindow( qt_xdisplay() ),
		   &dummy1, &dummy2, &dummy3, &dummy4, &dummy5, &dummy6,
		   &lmask );
    capsLocked = lmask & LockMask;
    updateStatus();
}

void
KGVerify::updateStatus()
{
    int nfls;

    if (!enabled)
	nfls = 0;
    else if (failed)
	nfls = 2;
    else if (!suspended && capsLocked)
	nfls = 1;
    else
	nfls = 0;

    if (failedLabelState != nfls ) {
	failedLabelState = nfls;
	switch (nfls) {
	default:
	    failedLabel->clear();
	    break;
	case 1:
	    failedLabel->setPaletteForegroundColor( Qt::red );
	    failedLabel->setText( i18n("Warning: Caps Lock on") );
	    break;
	case 2:
	    failedLabel->setPaletteForegroundColor( Qt::black );
	    failedLabel->setText( authTok ? i18n("Change failed") :
		    fixedEntity.isEmpty() ?
		      i18n("Login failed") : i18n("Authentication failed") );
	    break;
	}
    }
}

void
KGVerify::MsgBox( QMessageBox::Icon typ, const QString &msg )
{
    timer.suspend();
    FDialog::box( parent, typ, msg );
    timer.resume();
}


QVariant // public static
KGVerify::getConf( void *, const char *key, const QVariant &dflt )
{
    if (!qstrcmp( key, "EchoMode" ))
	return QVariant( kdmcfg->_echoMode );
    else {
	QString fkey = QString::fromLatin1( key ) + '=';
	for (QStringList::ConstIterator it = kdmcfg->_pluginOptions.begin();
	     it != kdmcfg->_pluginOptions.end(); ++it)
	    if ((*it).startsWith( fkey ))
		return (*it).mid( fkey.length() );
	return dflt;
    }
}

QValueVector<GreeterPluginHandle> KGVerify::greetPlugins;

PluginList
KGVerify::init( const QStringList &plugins )
{
    PluginList pluginList;

    for (QStringList::ConstIterator it = plugins.begin(); it != plugins.end(); ++it) {
	GreeterPluginHandle plugin;
	QString path = KLibLoader::self()->findLibrary(
		    ((*it)[0] == '/' ? *it : "kgreet_" + *it ).latin1() );
	if (path.isEmpty()) {
	    LogError( "GreeterPlugin %s does not exist\n", (*it).latin1() );
	    continue;
	}
	uint i, np = greetPlugins.count();
	for (i = 0; i < np; i++)
	    if (greetPlugins[i].library->fileName() == path)
		goto next;
	if (!(plugin.library = KLibLoader::self()->library( path.latin1() ))) {
	    LogError( "Cannot load GreeterPlugin %s (%s)\n", (*it).latin1(), path.latin1() );
	    continue;
	}
	if (!plugin.library->hasSymbol( "kgreeterplugin_info" )) {
	    LogError( "GreeterPlugin %s (%s) is no valid greet widget plugin\n",
		     (*it).latin1(), path.latin1() );
	    plugin.library->unload();
	    continue;
	}
	plugin.info = (kgreeterplugin_info*)plugin.library->symbol( "kgreeterplugin_info" );
	if (!plugin.info->init( QString::null, getConf, 0 )) {
	    LogError( "GreeterPlugin %s (%s) refuses to serve\n",
		     (*it).latin1(), path.latin1() );
	    plugin.library->unload();
	    continue;
        }
	Debug( "GreeterPlugin %s (%s) loaded\n", (*it).latin1(), plugin.info->name );
	greetPlugins.append( plugin );
      next:
	pluginList.append( i );
    }
    return pluginList;
}

void
KGVerify::done()
{
    for (uint i = 0; i < greetPlugins.count(); i++) {
	if (greetPlugins[i].info->done)
	    greetPlugins[i].info->done();
	greetPlugins[i].library->unload();
    }
}


KGChTok::KGChTok( QWidget *_parent, const QString &user,
		  const PluginList &pluginList, int curPlugin,
		  KGreeterPlugin::Function func,
		  KGreeterPlugin::Context ctx )
    : inherited( _parent )
    , verify( 0 )
{
    QSizePolicy fp( QSizePolicy::Fixed, QSizePolicy::Fixed );
    okButton = new KPushButton( KStdGuiItem::ok(), winFrame );
    okButton->setSizePolicy( fp );
    okButton->setDefault( true );
    cancelButton = new KPushButton( KStdGuiItem::cancel(), winFrame );
    cancelButton->setSizePolicy( fp );

    verify = new KGVerify( this, winFrame, cancelButton, user, pluginList, func, ctx );
    verify->selectPlugin( curPlugin );

    QVBoxLayout *box = new QVBoxLayout( winFrame, 10 );

    box->addWidget( new QLabel( i18n("Changing authentication token"), winFrame ), 0, AlignHCenter );

    box->addLayout( verify->getLayout() );

    box->addWidget( new KSeparator( KSeparator::HLine, winFrame ) );

    QHBoxLayout *hlay = new QHBoxLayout( box );
    hlay->addStretch( 1 );
    hlay->addWidget( okButton );
    hlay->addStretch( 1 );
    hlay->addWidget( cancelButton );
    hlay->addStretch( 1 );

    connect( okButton, SIGNAL(clicked()), SLOT(accept()) );
    connect( cancelButton, SIGNAL(clicked()), SLOT(reject()) );

    QTimer::singleShot( 0, verify, SLOT(start()) );
}

KGChTok::~KGChTok()
{
    hide();
    delete verify;
}

void
KGChTok::accept()
{
    verify->accept();
}

void
KGChTok::verifyPluginChanged( int )
{
    // cannot happen
}

void
KGChTok::verifyOk()
{
    inherited::accept();
}

void
KGChTok::verifyFailed()
{
    okButton->setEnabled( false );
    cancelButton->setEnabled( false );
}

void
KGChTok::verifyRetry()
{
    okButton->setEnabled( true );
    cancelButton->setEnabled( true );
}

void
KGChTok::verifySetUser( const QString & )
{
    // cannot happen
}


////// helper class, nuke when qtimer supports suspend()/resume()

QXTimer::QXTimer()
    : inherited( 0 )
    , left( -1 )
{
    connect( &timer, SIGNAL(timeout()), SLOT(slotTimeout()) );
}

void
QXTimer::start( int msec )
{
    left = msec;
    timer.start( left, true );
    gettimeofday( &stv, 0 );
}

void
QXTimer::stop()
{
    timer.stop();
    left = -1;
}

void
QXTimer::suspend()
{
    if (timer.isActive()) {
	timer.stop();
	struct timeval tv;
	gettimeofday( &tv, 0 );
	left -= (tv.tv_sec - stv.tv_sec) * 1000 + (tv.tv_usec - stv.tv_usec) / 1000;
	if (left < 0)
	    left = 0;
    }
}

void
QXTimer::resume()
{
    if (left >= 0 && !timer.isActive()) {
	timer.start( left, true );
	gettimeofday( &stv, 0 );
    }
}

void
QXTimer::slotTimeout()
{
    left = 0;
    emit timeout();
}


#include "kgverify.moc"
