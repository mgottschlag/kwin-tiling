/*

Shell for kdm conversation plugins

Copyright (C) 1997, 1998, 2000 Steffen Hansen <hansen@kde.org>
Copyright (C) 2000-2004 Oswald Buddenhagen <ossi@kde.org>


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
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

*/

#include <config.h>

#include "kgverify.h"
#include "kdmconfig.h"
#include "kdm_greet.h"

#include "themer/kdmthemer.h"
#include "themer/kdmitem.h"

#include <kapplication.h>
#include <klocale.h>
#include <klibloader.h>
#include <kseparator.h>
#include <kstdguiitem.h>
#include <kpushbutton.h>

#include <qregexp.h>
#include <QMenu>
#include <qlayout.h>
#include <qfile.h>
#include <qlabel.h>
//Added by qt3to4:
#include <QGridLayout>
#include <QEvent>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include <pwd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>

#include <X11/Xlib.h> // for updateLockStatus()
#include <fixx11h.h> // ... and make eventFilter() work again
#include <QX11Info>
#include <krandom.h>

#define FULL_GREET_TO 40 // normal inactivity timeout
#define TIMED_GREET_TO 20 // inactivity timeout when persisting timed login
#define MIN_TIMED_TO 5 // minimal timed login delay
#define DEAD_TIMED_TO 2 // <enter> dead time after re-activating timed login
#define SECONDS 1000 // reduce to 100 to speed up testing

void KGVerifyHandler::verifyClear()
{
}

void KGVerifyHandler::updateStatus( bool, bool, int )
{
}

KGVerify::KGVerify( KGVerifyHandler *_handler, KdmThemer *_themer,
                    QWidget *_parent, QWidget *_predecessor,
                    const QString &_fixedUser,
                    const PluginList &_pluginList,
                    KGreeterPlugin::Function _func,
                    KGreeterPlugin::Context _ctx )
	: inherited()
	, coreLock( 0 )
	, fixedEntity( _fixedUser )
	, pluginList( _pluginList )
	, handler( _handler )
	, themer( _themer )
	, parent( _parent )
	, predecessor( _predecessor )
	, plugMenu( 0 )
	, curPlugin( -1 )
	, timedLeft( 0 )
	, func( _func )
	, ctx( _ctx )
	, enabled( true )
	, running( false )
	, suspended( false )
	, failed( false )
	, isClear( true )
{
	connect( &timer, SIGNAL(timeout()), SLOT(slotTimeout()) );
	connect( kapp, SIGNAL(activity()), SLOT(slotActivity()) );

	_parent->installEventFilter( this );
}

KGVerify::~KGVerify()
{
	Debug( "delete %s\n", pName.data() );
	delete greet;
}

QMenu *
KGVerify::getPlugMenu()
{
	// assert( !cont );
	if (!plugMenu) {
		uint np = pluginList.count();
		if (np > 1) {
			plugMenu = new QMenu( parent );
			connect( plugMenu, SIGNAL(activated( int )),
			         SLOT(slotPluginSelected( int )) );
			for (uint i = 0; i < np; i++)
				plugMenu->insertItem( i18n(greetPlugins[pluginList[i]].info->name), pluginList[i] );
		}
	}
	return plugMenu;
}

bool // public
KGVerify::entitiesLocal() const
{
	return greetPlugins[pluginList[curPlugin]].info->flags & kgreeterplugin_info::Local;
}

bool // public
KGVerify::entitiesFielded() const
{
	return greetPlugins[pluginList[curPlugin]].info->flags & kgreeterplugin_info::Fielded;
}

bool // public
KGVerify::entityPresettable() const
{
	return greetPlugins[pluginList[curPlugin]].info->flags & kgreeterplugin_info::Presettable;
}

bool // public
KGVerify::isClassic() const
{
	return !strcmp( greetPlugins[pluginList[curPlugin]].info->method, "classic" );
}

QString // public
KGVerify::pluginName() const
{
	QString name( greetPlugins[pluginList[curPlugin]].library->fileName() );
	uint st = name.lastIndexOf( '/' ) + 1;
	uint en = name.indexOf( '.', st );
	if (en - st > 7 && QConstString( name.unicode() + st, 7 ).string() == "kgreet_")
		st += 7;
	return name.mid( st, en - st );
}

static void
showWidgets( QLayoutItem *li )
{
	QWidget *w;
	QLayout *l;

	if ((w = li->widget()))
		w->show();
	else if ((l = li->layout())) {
		QLayoutIterator it = l->iterator();
		for (QLayoutItem *itm = it.current(); itm; itm = ++it)
			 showWidgets( itm );
	}
}

void // public
KGVerify::selectPlugin( int id )
{
	if (pluginList.isEmpty()) {
		MsgBox( errorbox, i18n("No greeter widget plugin loaded. Check the configuration.") );
		::exit( EX_UNMANAGE_DPY );
	}
	curPlugin = id;
	if (plugMenu)
		plugMenu->setItemChecked( id, true );
	pName = ("greet_" + pluginName()).toLatin1();
	Debug( "new %s\n", pName.data() );
	greet = greetPlugins[pluginList[id]].info->create( this, themer, parent, predecessor, fixedEntity, func, ctx );
	timeable = _autoLoginDelay && entityPresettable() && isClassic();
}

void // public
KGVerify::loadUsers( const QStringList &users )
{
	Debug( "%s->loadUsers(...)\n", pName.data() );
	greet->loadUsers( users );
}

void // public
KGVerify::presetEntity( const QString &entity, int field )
{
	presEnt = entity;
	presFld = field;
}

bool // private
KGVerify::applyPreset()
{
	if (!presEnt.isEmpty()) {
		Debug( "%s->presetEntity(%\"s, %d)\n", pName.data(),
		       presEnt.toLatin1(), presFld );
		greet->presetEntity( presEnt, presFld );
		if (entitiesLocal()) {
			curUser = presEnt;
			handler->verifySetUser( presEnt );
		}
		return true;
	}
	return false;
}

bool // private
KGVerify::scheduleAutoLogin( bool initial )
{
	if (timeable) {
		Debug( "%s->presetEntity(%\"s, -1)\n", pName.data(),
		       _autoLoginUser.toLatin1(), -1 );
		greet->presetEntity( _autoLoginUser, -1 );
		curUser = _autoLoginUser;
		handler->verifySetUser( _autoLoginUser );
		timer.start( 1000 );
		if (initial) {
			timedLeft = _autoLoginDelay;
			deadTicks = 0;
		} else {
			timedLeft = qMax( _autoLoginDelay - TIMED_GREET_TO, MIN_TIMED_TO );
			deadTicks = DEAD_TIMED_TO;
		}
		updateStatus();
		running = false;
		isClear = true;
		return true;
	}
	return false;
}

void // private
KGVerify::performAutoLogin()
{
//	timer.stop();
	GSendInt( G_AutoLogin );
	handleVerify();
}

QString // public
KGVerify::getEntity() const
{
	Debug( "%s->getEntity()\n", pName.data() );
	QString ent = greet->getEntity();
	Debug( "  entity: %s\n", ent.toLatin1() );
	return ent;
}

void
KGVerify::setUser( const QString &user )
{
	// assert( fixedEntity.isEmpty() );
	curUser = user;
	Debug( "%s->setUser(%\"s)\n", pName.data(), user.toLatin1() );
	greet->setUser( user );
	gplugActivity();
}

void
KGVerify::start()
{
	authTok = (func == KGreeterPlugin::ChAuthTok);
	cont = false;
	if (func == KGreeterPlugin::Authenticate) {
		if (scheduleAutoLogin( true )) {
			if (!_autoLoginAgain)
				_autoLoginDelay = 0, timeable = false;
			return;
		} else
			applyPreset();
	}
	running = true;
	Debug( "%s->start()\n", pName.data() );
	greet->start();
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
	Debug( "%s->abort()\n", pName.data() );
	greet->abort();
	running = false;
}

void
KGVerify::suspend()
{
	// assert( !cont );
	if (running) {
		Debug( "%s->abort()\n", pName.data() );
		greet->abort();
	}
	suspended = true;
	updateStatus();
	timer.suspend();
}

void
KGVerify::resume()
{
	timer.resume();
	suspended = false;
	updateLockStatus();
	if (running) {
		Debug( "%s->start()\n", pName.data() );
		greet->start();
	} else if (delayed) {
		delayed = false;
		running = true;
		Debug( "%s->start()\n", pName.data() );
		greet->start();
	}
}

void // not a slot - called manually by greeter
KGVerify::accept()
{
	Debug( "%s->next()\n", pName.data() );
	greet->next();
}

void // private
KGVerify::doReject( bool initial )
{
	// assert( !cont );
	if (running) {
		Debug( "%s->abort()\n", pName.data() );
		greet->abort();
	}
	handler->verifyClear();
	Debug( "%s->clear()\n", pName.data() );
	greet->clear();
	curUser.clear();
	if (!scheduleAutoLogin( initial )) {
		isClear = !(isClear && applyPreset());
		if (running) {
			Debug( "%s->start()\n", pName.data() );
			greet->start();
		}
		if (!failed)
			timer.stop();
	}
}

void // not a slot - called manually by greeter
KGVerify::reject()
{
	doReject( true );
}

void
KGVerify::setEnabled( bool on )
{
	Debug( "%s->setEnabled(%s)\n", pName.data(), on ? "true" : "false" );
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
		Debug( "%s->revive()\n", pName.data() );
		greet->revive();
		handler->verifyRetry();
		if (suspended)
			delayed = true;
		else {
			running = true;
			Debug( "%s->start()\n", pName.data() );
			greet->start();
			slotActivity();
			gplugActivity();
			if (cont)
				handleVerify();
		}
	} else if (timedLeft) {
		deadTicks--;
		if (!--timedLeft)
			performAutoLogin();
		else
			timer.start( 1000 );
		updateStatus();
	} else {
		// assert( ctx == Login );
		isClear = true;
		doReject( false );
	}
}

void
KGVerify::slotActivity()
{
	if (timedLeft) {
		Debug( "%s->revive()\n", pName.data() );
		greet->revive();
		Debug( "%s->start()\n", pName.data() );
		greet->start();
		running = true;
		timedLeft = 0;
		updateStatus();
		timer.start( TIMED_GREET_TO * SECONDS );
	} else if (timeable)
		timer.start( TIMED_GREET_TO * SECONDS );
}


void // private static
KGVerify::VMsgBox( QWidget *parent, const QString &user,
                   QMessageBox::Icon type, const QString &mesg )
{
	FDialog::box( parent, type, user.isEmpty() ?
	              mesg : i18n("Authenticating %1 ...\n\n", user ) + mesg );
}

static const char *msgs[]= {
	I18N_NOOP( "You are required to change your password immediately (password aged)." ),
	I18N_NOOP( "You are required to change your password immediately (root enforced)." ),
	I18N_NOOP( "You are not allowed to login at the moment." ),
	I18N_NOOP( "Home folder not available." ),
	I18N_NOOP( "Logins are not allowed at the moment.\nTry again later." ),
	I18N_NOOP( "Your login shell is not listed in /etc/shells." ),
	I18N_NOOP( "Root logins are not allowed." ),
	I18N_NOOP( "Your account has expired; please contact your system administrator." )
};

void // private static
KGVerify::VErrBox( QWidget *parent, const QString &user, const char *msg )
{
	QMessageBox::Icon icon;
	QString mesg;

	if (!msg) {
		mesg = i18n("A critical error occurred.\n"
		            "Please look at KDM's logfile(s) for more information\n"
		            "or contact your system administrator.");
		icon = errorbox;
	} else {
		mesg = QString::fromLocal8Bit( msg );
		QString mesg1 = mesg + '.';
		for (uint i = 0; i < as(msgs); i++)
			if (mesg1 == msgs[i]) {
				mesg = i18n(msgs[i]);
				break;
			}
		icon = sorrybox;
	}
	VMsgBox( parent, user, icon, mesg );
}

void // private static
KGVerify::VInfoBox( QWidget *parent, const QString &user, const char *msg )
{
	QString mesg = QString::fromLocal8Bit( msg );
	QRegExp rx( "^Warning: your account will expire in (\\d+) day" );
	if (rx.indexIn( mesg ) >= 0) {
		int expire = rx.cap( 1 ).toInt();
		mesg = expire ?
			i18np("Your account expires tomorrow.",
			     "Your account expires in %n days.", expire) :
			i18n("Your account expires today.");
	} else {
		rx.setPattern( "^Warning: your password will expire in (\\d+) day" );
		if (rx.indexIn( mesg ) >= 0) {
			int expire = rx.cap( 1 ).toInt();
			mesg = expire ?
				i18np("Your password expires tomorrow.",
				     "Your password expires in %n days.", expire) :
				i18n("Your password expires today.");
		}
	}
	VMsgBox( parent, user, infobox, mesg );
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
				QStringList pgs( _pluginsLogin );
				pgs += _pluginsShutdown;
				QStringList::ConstIterator it;
				for (it = pgs.begin(); it != pgs.end(); ++it)
					if (*it == "classic" || *it == "modern") {
						pgs = QStringList(*it);
						goto gotit;
					} else if (*it == "generic") {
						pgs = QStringList("modern");
						goto gotit;
					}
				pgs = QStringList("classic");
			  gotit:
				KGChTok chtok( parent, user, init( pgs ), 0,
				               KGreeterPlugin::AuthChAuthTok,
				               KGreeterPlugin::Login );
				return chtok.exec();
			}
		case V_MSG_ERR:
			Debug( " V_MSG_ERR\n" );
			msg = GRecvStr();
			Debug( "  message %\"s\n", msg );
			VErrBox( parent, user, msg );
			if (msg)
				free( msg );
			continue;
		case V_MSG_INFO:
			Debug( " V_MSG_INFO\n" );
			msg = GRecvStr();
			Debug( "  message %\"s\n", msg );
			VInfoBox( parent, user, msg );
			free( msg );
			continue;
		}

		// terminal status
		switch (ret) {
		case V_OK:
			Debug( " V_OK\n" );
			return true;
		case V_AUTH:
			Debug( " V_AUTH\n" );
			VMsgBox( parent, user, sorrybox, i18n("Authentication failed") );
			return false;
		case V_FAIL:
			Debug( " V_FAIL\n" );
			return false;
		default:
			LogPanic( "Unknown V_xxx code %d from core\n", ret );
		}
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
			Debug( "  ndelay = %d\n%s->textPrompt(...)\n", ndelay, pName.data() );
			greet->textPrompt( msg, echo, ndelay );
			if (msg)
				free( msg );
			return;
		case V_GET_BINARY:
			Debug( " V_GET_BINARY\n" );
			msg = GRecvArr( &ret );
			Debug( "  %d bytes prompt\n", ret );
			ndelay = GRecvInt();
			Debug( "  ndelay = %d\n%s->binaryPrompt(...)\n", ndelay, pName.data() );
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
			curUser = user = QString::fromLocal8Bit( msg );
			// greet needs this to be able to return something useful from
			// getEntity(). but the backend is still unable to tell a domain ...
			Debug( "  %s->setUser(%\"s)\n", pName.data(), user.toLatin1() );
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
			Debug( "%s->succeeded()\n", pName.data() );
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
			user.clear();
		  dchtok:
			{
				timer.stop();
				Debug( "%s->succeeded()\n", pName.data() );
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
			Debug( "  %s->textMessage(%\"s, true)\n", pName.data(), msg );
			if (!greet->textMessage( msg, true )) {
				Debug( "  message passed\n" );
				VErrBox( parent, user, msg );
			} else
				Debug( "  message swallowed\n" );
			if (msg)
				free( msg );
			continue;
		case V_MSG_INFO:
			Debug( " V_MSG_INFO\n" );
			msg = GRecvStr();
			Debug( "  %s->textMessage(%\"s, false)\n", pName.data(), msg );
			if (!greet->textMessage( msg, false )) {
				Debug( "  message passed\n" );
				VInfoBox( parent, user, msg );
			} else
				Debug( "  message swallowed\n" );
			free( msg );
			continue;
		}

		// terminal status
		coreLock = 0;
		running = false;
		timer.stop();

		if (ret == V_OK) {
			Debug( " V_OK\n" );
			if (!fixedEntity.isEmpty()) {
				Debug( "  %s->getEntity()\n", pName.data() );
				QString ent = greet->getEntity();
				Debug( "  entity %\"s\n", ent.toLatin1() );
				if (ent != fixedEntity) {
					Debug( "%s->failed()\n", pName.data() );
					greet->failed();
					MsgBox( sorrybox,
					        i18n("Authenticated user (%1) does not match requested user (%2).\n",
					          ent, fixedEntity ) );
					goto retry;
				}
			}
			Debug( "%s->succeeded()\n", pName.data() );
			greet->succeeded();
			handler->verifyOk();
			return;
		}

		Debug( "%s->failed()\n", pName.data() );
		greet->failed();

		if (ret == V_AUTH) {
			Debug( " V_AUTH\n" );
			failed = true;
			updateStatus();
			handler->verifyFailed();
			timer.start( 1500 + KRandom::random()/(RAND_MAX/1000) );
			return;
		}
		if (ret != V_FAIL)
			LogPanic( "Unknown V_xxx code %d from core\n", ret );
		Debug( " V_FAIL\n" );
	  retry:
		Debug( "%s->revive()\n", pName.data() );
		greet->revive();
		running = true;
		Debug( "%s->start()\n", pName.data() );
		greet->start();
		if (!cont)
			return;
		user.clear();
	}
}

void
KGVerify::gplugReturnText( const char *text, int tag )
{
	Debug( "%s: gplugReturnText(%\"s, %d)\n", pName.data(),
	       tag & V_IS_SECRET ? "<masked>" : text, tag );
	GSendStr( text );
	if (text) {
		GSendInt( tag );
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
		Debug( "%s: gplugReturnBinary(%d bytes)\n", pName.data(), len );
		GSendArr( len, data );
		handleVerify();
	} else {
		Debug( "%s: gplugReturnBinary(NULL)\n", pName.data() );
		GSendArr( 0, 0 );
		coreLock = 0;
	}
}

void
KGVerify::gplugSetUser( const QString &user )
{
	Debug( "%s: gplugSetUser(%\"s)\n", pName.data(), user.toLatin1() );
	curUser = user;
	handler->verifySetUser( user );
}

void
KGVerify::gplugStart()
{
	// XXX handle func != Authenticate
	Debug( "%s: gplugStart()\n", pName.data() );
	GSendInt( ctx == KGreeterPlugin::Shutdown ? G_VerifyRootOK : G_Verify );
	GSendStr( greetPlugins[pluginList[curPlugin]].info->method );
	handleVerify();
}

void
KGVerify::gplugActivity()
{
	Debug( "%s: gplugActivity()\n", pName.data() );
	if (func == KGreeterPlugin::Authenticate &&
	    ctx == KGreeterPlugin::Login)
	{
		isClear = false;
		if (!timeable)
			timer.start( FULL_GREET_TO * SECONDS );
	}
}

void
KGVerify::gplugMsgBox( QMessageBox::Icon type, const QString &text )
{
	Debug( "%s: gplugMsgBox(%d, %\"s)\n", pName.data(), type, text.toLatin1() );
	MsgBox( type, text );
}

bool
KGVerify::eventFilter( QObject *o, QEvent *e )
{
	switch (e->type()) {
	case QEvent::KeyPress:
		if (timedLeft) {
			QKeyEvent *ke = (QKeyEvent *)e;
			if (ke->key() == Qt::Key_Return || ke->key() == Qt::Key_Enter) {
				if (deadTicks <= 0) {
					timedLeft = 0;
					performAutoLogin();
				}
				return true;
			}
		}
		/* fall through */
	case QEvent::KeyRelease:
		updateLockStatus();
		/* fall through */
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
	XQueryPointer( QX11Info::display(), DefaultRootWindow( QX11Info::display() ),
	               &dummy1, &dummy2, &dummy3, &dummy4, &dummy5, &dummy6,
	               &lmask );
	capsLocked = lmask & LockMask;
	updateStatus();
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
		return QVariant( _echoMode );
	else {
		QString fkey = QString::fromLatin1( key ) + '=';
		for (QStringList::ConstIterator it = _pluginOptions.begin();
		     it != _pluginOptions.end(); ++it)
			if ((*it).startsWith( fkey ))
				return (*it).mid( fkey.length() );
		return dflt;
	}
}

QVector<GreeterPluginHandle> KGVerify::greetPlugins;

PluginList
KGVerify::init( const QStringList &plugins )
{
	PluginList pluginList;

	for (QStringList::ConstIterator it = plugins.begin(); it != plugins.end(); ++it) {
		GreeterPluginHandle plugin;
		QString path = KLibLoader::self()->findLibrary(
			((*it)[0] == '/' ? *it : "kgreet_" + *it ).toLatin1() );
		if (path.isEmpty()) {
			LogError( "GreeterPlugin %s does not exist\n", (*it).toLatin1() );
			continue;
		}
		uint i, np = greetPlugins.count();
		for (i = 0; i < np; i++)
			if (greetPlugins[i].library->fileName() == path)
				goto next;
		if (!(plugin.library = KLibLoader::self()->library( path.toLatin1() ))) {
			LogError( "Cannot load GreeterPlugin %s (%s)\n", (*it).toLatin1(), path.toLatin1() );
			continue;
		}
		if (!plugin.library->hasSymbol( "kgreeterplugin_info" )) {
			LogError( "GreeterPlugin %s (%s) is no valid greet widget plugin\n",
			          (*it).toLatin1(), path.toLatin1() );
			plugin.library->unload();
			continue;
		}
		plugin.info = (kgreeterplugin_info*)plugin.library->symbol( "kgreeterplugin_info" );
		if (!plugin.info->init( QString(), getConf, 0 )) {
			LogError( "GreeterPlugin %s (%s) refuses to serve\n",
			          (*it).toLatin1(), path.toLatin1() );
			plugin.library->unload();
			continue;
		}
		Debug( "GreeterPlugin %s (%s) loaded\n", (*it).toLatin1(), plugin.info->name );
		greetPlugins.append( plugin );
	  next:
		pluginList.append( i );
	}
	return pluginList;
}

void
KGVerify::done()
{
	for (int i = 0; i < greetPlugins.count(); i++) {
		if (greetPlugins[i].info->done)
			greetPlugins[i].info->done();
		greetPlugins[i].library->unload();
	}
}


KGStdVerify::KGStdVerify( KGVerifyHandler *_handler, QWidget *_parent,
                          QWidget *_predecessor, const QString &_fixedUser,
                          const PluginList &_pluginList,
                          KGreeterPlugin::Function _func,
                          KGreeterPlugin::Context _ctx )
	: inherited( _handler, 0, _parent, _predecessor, _fixedUser,
	             _pluginList, _func, _ctx )
	, failedLabelState( 0 )
{
	grid = new QGridLayout;
	grid->setAlignment( Qt::AlignCenter );

	failedLabel = new QLabel( parent );
	failedLabel->setFont( _failFont );
	grid->addWidget( failedLabel, 1, 0, Qt::AlignCenter );

	updateLockStatus();
}

KGStdVerify::~KGStdVerify()
{
	grid->removeItem( greet->getLayoutItem() );
}

void // public
KGStdVerify::selectPlugin( int id )
{
	inherited::selectPlugin( id );
	grid->addItem( greet->getLayoutItem(), 0, 0 );
	showWidgets( greet->getLayoutItem() );
}

void // private slot
KGStdVerify::slotPluginSelected( int id )
{
	if (failed)
		return;
	if (id != curPlugin) {
		plugMenu->setItemChecked( curPlugin, false );
		parent->setUpdatesEnabled( false );
		grid->removeItem( greet->getLayoutItem() );
		Debug( "delete %s\n", pName.data() );
		delete greet;
		selectPlugin( id );
		handler->verifyPluginChanged( id );
		if (running)
			start();
		parent->setUpdatesEnabled( true );
	}
}

void
KGStdVerify::updateStatus()
{
	int nfls;

	if (!enabled)
		nfls = 1;
	else if (failed)
		nfls = 2;
	else if (timedLeft)
		nfls = -timedLeft;
	else if (!suspended && capsLocked)
		nfls = 3;
	else
		nfls = 1;

	if (failedLabelState != nfls) {
		failedLabelState = nfls;
		if (nfls < 0) {
			failedLabel->setPaletteForegroundColor( Qt::black );
			failedLabel->setText( i18np( "Automatic login in 1 second ...",
			                            "Automatic login in %n seconds ...",
			                            timedLeft ) );
		} else {
			switch (nfls) {
			default:
				failedLabel->clear();
				break;
			case 3:
				failedLabel->setPaletteForegroundColor( Qt::red );
				failedLabel->setText( i18n("Warning: Caps Lock on") );
				break;
			case 2:
				failedLabel->setPaletteForegroundColor( Qt::black );
				failedLabel->setText( authTok ?
				                         i18n("Change failed") :
				                         fixedEntity.isEmpty() ?
				                            i18n("Login failed") :
				                            i18n("Authentication failed") );
				break;
			}
		}
	}
}

KGThemedVerify::KGThemedVerify( KGVerifyHandler *_handler,
                                KdmThemer *_themer,
                                QWidget *_parent, QWidget *_predecessor,
                                const QString &_fixedUser,
                                const PluginList &_pluginList,
                                KGreeterPlugin::Function _func,
                                KGreeterPlugin::Context _ctx )
	: inherited( _handler, _themer, _parent, _predecessor, _fixedUser,
	             _pluginList, _func, _ctx )
{
	updateLockStatus();
}

KGThemedVerify::~KGThemedVerify()
{
}

void // public
KGThemedVerify::selectPlugin( int id )
{
	inherited::selectPlugin( id );
	QLayoutItem *l;
	KdmItem *n;
	if (themer && (l = greet->getLayoutItem())) {
		if (!(n = themer->findNode( "talker" )))
			MsgBox( errorbox,
			        i18n("Theme not usable with authentication method '%1'.",
			          i18n(greetPlugins[pluginList[id]].info->name) ) );
		else {
			n->setLayoutItem( l );
			showWidgets( l );
		}
	}
	if (themer)
		themer->updateGeometry( true );
}

void // private slot
KGThemedVerify::slotPluginSelected( int id )
{
	if (failed)
		return;
	if (id != curPlugin) {
		plugMenu->setItemChecked( curPlugin, false );
		Debug( "delete %s\n", pName.data() );
		delete greet;
		selectPlugin( id );
		handler->verifyPluginChanged( id );
		if (running)
			start();
	}
}

void
KGThemedVerify::updateStatus()
{
	handler->updateStatus( enabled && failed,
	                       enabled && !suspended && capsLocked,
	                       timedLeft );
}


KGChTok::KGChTok( QWidget *_parent, const QString &user,
                  const PluginList &pluginList, int curPlugin,
                  KGreeterPlugin::Function func,
                  KGreeterPlugin::Context ctx )
	: inherited( _parent )
	, verify( 0 )
{
	QSizePolicy fp( QSizePolicy::Fixed, QSizePolicy::Fixed );
	okButton = new KPushButton( KStdGuiItem::ok(), this );
	okButton->setSizePolicy( fp );
	okButton->setDefault( true );
	cancelButton = new KPushButton( KStdGuiItem::cancel(), this );
	cancelButton->setSizePolicy( fp );

	verify = new KGStdVerify( this, this, cancelButton, user, pluginList, func, ctx );
	verify->selectPlugin( curPlugin );

	QVBoxLayout *box = new QVBoxLayout( this );
	box->setSpacing( 10 );

	box->addWidget( new QLabel( i18n("Changing authentication token"), this ), 0, Qt::AlignHCenter );

	box->addLayout( verify->getLayout() );

	box->addWidget( new KSeparator( Qt::Horizontal, this ) );

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
	timer.setSingleShot( true );
	timer.start( left );
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
		timer.setSingleShot( true );
		timer.start( left );
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
