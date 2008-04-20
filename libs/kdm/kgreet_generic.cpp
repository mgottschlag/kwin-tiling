/*

Conversation widget for kdm greeter

Copyright (C) 2008 Dirk Mueller <mueller@kde.org>

based on classic kdm greeter:

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
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

*/

#include "kgreet_generic.h"
#include "themer/kdmthemer.h"
#include "themer/kdmlabel.h"

#include <kglobal.h>
#include <klocale.h>
#include <klineedit.h>
#include <kuser.h>

#include <qregexp.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qtimer.h>

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <syslog.h>

//#define PAM_GREETER_DEBUG

class KDMPasswordEdit : public KLineEdit {
public:
	KDMPasswordEdit( QWidget *parent ) : KLineEdit( parent ) {}
	KDMPasswordEdit( bool echoMode, QWidget *parent ) : KLineEdit( parent )
	{
		setEchoMode( echoMode ? Password : NoEcho );
	}
protected:
	virtual void contextMenuEvent( QContextMenuEvent * ) {}
};

static FILE* log;
static void debug( const char* fmt, ... )
{
	va_list lst;
	va_start( lst, fmt );

#ifdef PAM_GREETER_DEBUG
# if 0
	vfprintf( log, fmt, lst );
	fflush( log );
# else
	char buf[6000];
	sprintf( buf, "*** %s\n", fmt );
	vsyslog( LOG_WARNING, buf, lst );
# endif
#endif
	va_end( lst );
}

static bool echoMode;

KGenericGreeter::KGenericGreeter( KGreeterPluginHandler *_handler,
                                  QWidget *parent,
                                  const QString &_fixedEntity,
                                  Function _func, Context _ctx ) :
	QObject(),
	KGreeterPlugin( _handler ),
	fixedUser( _fixedEntity ),
	func( _func ),
	ctx( _ctx ),
	exp( -1 ),
	pExp( -1 ),
	running( false )
{
	ctx = Login;

	debug( "KGenericGreeter constructed\n" );

	m_parentWidget = parent;

	KdmItem *user_entry = 0, *pw_entry = 0;
	int line = 0;
	QGridLayout *grid = 0;

	if (!_handler->gplugHasNode( "user-entry" ) || !_handler->gplugHasNode( "pw-entry" )) {
		parent = new QWidget( parent );
		parent->setObjectName( "talker" );
		widgetList << parent;
		grid = new QGridLayout( parent );
		grid->setMargin( 0 );
	}

	loginLabel = 0;
	authLabel.clear();
	authEdit.clear();
	loginLabel = 0;
	loginEdit = 0;
	if (ctx == ExUnlock || ctx == ExChangeTok)
		fixedUser = KUser().loginName();
	if (func != ChAuthTok) {
		debug( "func != ChAuthTok\n" );

		if (fixedUser.isEmpty()) {
			loginEdit = new KLineEdit( parent );
			loginEdit->setContextMenuEnabled( false );
			connect( loginEdit, SIGNAL(lostFocus()), SLOT(slotLoginLostFocus()) );
			connect( loginEdit, SIGNAL(lostFocus()), SLOT(slotActivity()) );
			connect( loginEdit, SIGNAL(textChanged( const QString & )), SLOT(slotActivity()) );
			connect( loginEdit, SIGNAL(selectionChanged()), SLOT(slotActivity()) );
			if (!getLayoutItem()) {
				loginEdit->adjustSize();
				user_entry->setWidget( loginEdit );
			} else {
				loginLabel = new QLabel( i18n("Username:"), parent );
				loginLabel->setBuddy( loginEdit );

				getLayoutItem()->addWidget( loginLabel, line, 0 );
				getLayoutItem()->addWidget( loginEdit, line++, 1 );
			}
		} else if (ctx != Login && ctx != Shutdown && getLayoutItem()) {
			loginLabel = new QLabel( i18n("Username:"), parent );
			getLayoutItem()->addWidget( loginLabel, line, 0 );
			getLayoutItem()->addWidget( new QLabel( fixedUser, parent ), line++, 1 );
		}
#if 0
		if (echoMode == -1)
			passwdEdit = new KDMPasswordEdit( parent );
		else
			passwdEdit = new KDMPasswordEdit( echoMode,
			                                  parent );
		connect( passwdEdit, SIGNAL(textChanged( const QString & )),
		         SLOT(slotActivity()) );
		connect( passwdEdit, SIGNAL(lostFocus()), SLOT(slotActivity()) );
		if (pred) {
			parent->setTabOrder( pred, passwdEdit );
			pred = passwdEdit;
		}
		if (!getLayoutItem()) {
			passwdEdit->adjustSize();
			pw_entry->setWidget( passwdEdit );
		} else {
			passwdLabel = new QLabel( passwdEdit,
			                          func == Authenticate ?
			                          i18n("hello &Password:") :
			                          i18n("Current &password:"),
			                          parent );
			getLayoutItem()->addWidget( passwdLabel, line, 0 );
			getLayoutItem()->addWidget( passwdEdit, line++, 1 );
		}
#endif
		if (loginEdit)
			loginEdit->setFocus();
	}
	if (func != Authenticate) {
		if (echoMode == -1) {
			authEdit << new KDMPasswordEdit( echoMode, parent );
			authEdit << new KDMPasswordEdit( echoMode, parent );
		} else {
			authEdit << new KDMPasswordEdit( parent );
			authEdit << new KDMPasswordEdit( parent );
		}
		QLabel* l = new QLabel( i18n("&New password:"), parent );
		authLabel << l;
		l->setBuddy( authEdit[0] );
		l = new QLabel( i18n("Con&firm password:"), parent );
		authLabel << l;
		l->setBuddy( authEdit[1] );

		if (getLayoutItem()) {
			getLayoutItem()->addWidget( authLabel[0], line, 0 );
			getLayoutItem()->addWidget( authEdit[0], line++, 1 );
			getLayoutItem()->addWidget( authLabel[1], line, 0 );
			getLayoutItem()->addWidget( authEdit[1], line, 1 );
		}
		if (authEdit.size() >= 2)
			authEdit[1]->setFocus();
	}
}

// virtual
KGenericGreeter::~KGenericGreeter()
{
	debug( "KGenericGreeter::~KPamGreeter" );
	abort();
#if 0
	if (!layoutItem) {
		delete loginEdit;
		return;
	}
	QLayoutIterator it = static_cast<QLayout *>(layoutItem)->iterator();
	for (QLayoutItem *itm = it.current(); itm; itm = ++it)
		 delete itm->widget();
	delete layoutItem;
#endif
	debug( "destructor finished, good bye" );
}

void // virtual
KGenericGreeter::loadUsers( const QStringList &users )
{
	KCompletion *userNamesCompletion = new KCompletion;
	userNamesCompletion->setItems( users );
	loginEdit->setCompletionObject( userNamesCompletion );
	loginEdit->setAutoDeleteCompletionObject( true );
	loginEdit->setCompletionMode( KGlobalSettings::CompletionAuto );
}

void // virtual
KGenericGreeter::presetEntity( const QString &entity, int field )
{
	debug( "presetEntity(%s,%d) called!\n", qPrintable( entity ), field );
	loginEdit->setText( entity );
	if (field == 1 && authEdit.size() >= 1)
		authEdit[0]->setFocus();
	else {
		loginEdit->setFocus();
		loginEdit->selectAll();
		if (field == -1 && authEdit.size() >= 1) {
			authEdit[0]->setText( "     " );
			authEdit[0]->setEnabled( false );
			authTok = false;
		}
	}
	curUser = entity;
}

QString // virtual
KGenericGreeter::getEntity() const
{
	return fixedUser.isEmpty() ? loginEdit->text() : fixedUser;
}

void // virtual
KGenericGreeter::setUser( const QString &user )
{
	// assert( fixedUser.isEmpty() );
	curUser = user;
	loginEdit->setText( user );
	if (authEdit.size() >= 1) {
		authEdit[0]->setFocus();
		authEdit[0]->selectAll();
	}
}

void // virtual
KGenericGreeter::setEnabled( bool enable )
{
	// assert( !passwd1Label );
	// assert( func == Authenticate && ctx == Shutdown );
//	if (loginLabel)
//		loginLabel->setEnabled( enable );
	authEdit[0]->setEnabled( enable );
	setActive( enable );
	if (enable)
		authEdit[0]->setFocus();
}

void // private
KGenericGreeter::returnData()
{
	debug( "*************** returnData called with exp %d\n", exp );

	switch (exp) {
	case 0:
		handler->gplugReturnText( (loginEdit ? loginEdit->text() :
		                                       fixedUser).toLocal8Bit(),
		                          KGreeterPluginHandler::IsUser );
		break;
	case 1:
		handler->gplugReturnText( authEdit[0]->text().toLocal8Bit(),
		                          KGreeterPluginHandler::IsPassword |
		                          KGreeterPluginHandler::IsSecret );
		break;
	case 2:
		handler->gplugReturnText( authEdit[1]->text().toLocal8Bit(),
		                          KGreeterPluginHandler::IsSecret );
		break;
	default: // case 3:
		handler->gplugReturnText( authEdit[2]->text().toLocal8Bit(),
		                          KGreeterPluginHandler::IsNewPassword |
		                          KGreeterPluginHandler::IsSecret );
		break;
	}
}

bool // virtual
KGenericGreeter::textMessage( const char *text, bool err )
{
	debug( " ************** textMessage(%s, %d)\n", text, err );

	if (!authEdit.size())
		return false;

	if (getLayoutItem()) {
		QLabel* label = new QLabel( QString::fromUtf8( text ), m_parentWidget );
		getLayoutItem()->addWidget( label, state+1, 0, 0 );
	}

	return true;
}

void // virtual
KGenericGreeter::textPrompt( const char *prompt, bool echo, bool nonBlocking )
{
	debug( "textPrompt called with prompt %s echo %d nonBlocking %d", prompt, echo, nonBlocking );
	debug( "state is %d, authEdit.size is %d\n", state, authEdit.size() );

	if (state == 0 && echo) {
#ifdef PORTED
		if (loginLabel)
			loginLabel->setText( QString::fromUtf8( prompt ) );
		else if (m_themer) {
			KdmLabel *kdmlabel = static_cast<KdmLabel*>(m_themer->findNode( "user-label" ));
			if (kdmlabel) {
				//userLabel->setText(QString::fromUtf8(prompt));
				kdmlabel->label.text = QString::fromUtf8( prompt );
				QTimer::singleShot( 0, kdmlabel, SLOT(update()) );
			}
		}
#endif
	} else if (state >= authEdit.size()) {
#ifdef PORTED
		if (getLayoutItem()) {
			QLabel* label = new QLabel( QString::fromUtf8( prompt ), m_parentWidget );
			getLayoutItem()->addWidget( label, state+1, 0, 0 );
			debug( "added label widget to layout" );
		} else if (m_themer) {
			debug( "themer found!" );
			KdmItem *pw_label = 0;

			KdmLabel *kdmlabel = static_cast<KdmLabel*>(m_themer->findNode( "pw-label" ));
			if (kdmlabel) {
				//userLabel->setText(QString::fromUtf8(prompt));
				QString str = QString::fromUtf8( prompt );
				kdmlabel->label.text = str;
				QTimer::singleShot( 0, kdmlabel, SLOT(update()) );
			}
		}
#endif

		KDMPasswordEdit* passwdEdit;

		if (echoMode == -1)
			passwdEdit = new KDMPasswordEdit( m_parentWidget );
		else
			passwdEdit = new KDMPasswordEdit( echoMode, m_parentWidget );
		connect( passwdEdit, SIGNAL(textChanged( const QString & )),
		         SLOT(slotActivity()) );
		connect( passwdEdit, SIGNAL(lostFocus()), SLOT(slotActivity()) );
		authEdit << passwdEdit;

#if 1
		for (QList<KLineEdit*>::iterator it = authEdit.begin();
		     it != authEdit.end();
		     ++it)
		{
			if ((*it)->isEnabled() && (*it)->text().isEmpty()) {
				(*it)->setFocus();
				break;
			}
		}
#endif
		if (getLayoutItem())
			getLayoutItem()->addWidget( passwdEdit, state+1, 1, 0 );

#ifdef PORTED
		if (m_themer) {
			debug( "themer found!" );
			KdmItem *pw_entry = 0;

			pw_entry = m_themer->findNode( "pw-entry" );

			if (pw_entry && passwdEdit)
				pw_entry->setWidget( passwdEdit );

				if (0) {
					//userLabel->setText(QString::fromUtf8(prompt));
					//kdmlabel->label.text = QString::fromUtf8(prompt);
					//QTimer::singleShot(0, kdmlabel, SLOT(update()));
				}
		} else
			debug( "no themer found!" );
#endif
	}
	++state;
	pExp = exp;

	exp = authEdit.size();
	debug( "state %d exp: %d, has %d\n", state, exp, has );

	if (has >= exp || nonBlocking)
		returnData();
}

bool // virtual
KGenericGreeter::binaryPrompt( const char *, bool )
{
	// this simply cannot happen ... :}
	return true;
}

void // virtual
KGenericGreeter::start()
{
	debug( "******* start() called\n" );

	qDeleteAll( authEdit );
	authEdit.clear();

	qDeleteAll( authLabel );
	authLabel.clear();

	authTok = !(authEdit.size() >= 2 && authEdit[1]->isEnabled());
	exp = has = -1;
	state = 0;
	running = true;
	handler->gplugStart();
}

void // virtual
KGenericGreeter::suspend()
{
}

void // virtual
KGenericGreeter::resume()
{
}

void // virtual
KGenericGreeter::next()
{
	debug( "********* next() called state %d\n", state );

	if (state == 0 && running && handler) {
		debug( " **** returned text!\n" );
		handler->gplugReturnText( (loginEdit ? loginEdit->text() : fixedUser).toLocal8Bit(),
		                          KGreeterPluginHandler::IsUser );
		setActive( false );
	}

	has = 0;

	for (QList<KLineEdit*>::iterator it = authEdit.begin();
	     it != authEdit.end();
	     ++it)
	{
		has++;
		if ((*it)->hasFocus()) {
			++it;
			if (it != authEdit.end())
				(*it)->setFocus();
			break;
		}
		if (it == authEdit.end())
			has = -1;
	}

	debug( " has %d and exp %d\n", has, exp );

#if 0
	// assert( running );
	if (loginEdit && loginEdit->hasFocus()) {
		passwdEdit->setFocus(); // will cancel running login if necessary
		has = 0;
	} else if (passwdEdit && passwdEdit->hasFocus()) {
		if (passwd1Edit)
			passwd1Edit->setFocus();
		has = 1;
	} else if (passwd1Edit) {
		if (passwd1Edit->hasFocus()) {
			passwd2Edit->setFocus();
			has = 1; // sic!
		} else
			has = 3;
	} else
		has = 1;
	if (exp < 0)
		handler->gplugStart();
#endif
	if (has >= exp)
		returnData();
}

void // virtual
KGenericGreeter::abort()
{
	debug( "***** abort() called\n" );

	running = false;
	if (exp >= 0) {
		exp = -1;
		handler->gplugReturnText( 0, 0 );
	}
}

void // virtual
KGenericGreeter::succeeded()
{
	debug( "**** succeeded() called\n" );

	// assert( running || timed_login );
	if (!authTok)
		setActive( false );
	else
		setAllActive( false );
	exp = -1;
	running = false;
}

void // virtual
KGenericGreeter::failed()
{
	// assert( running || timed_login );
	setActive( false );
	setAllActive( false );
	exp = -1;
	running = false;
}

#include<assert.h>
void // virtual
KGenericGreeter::revive()
{
	// assert( !running );
	setAllActive( true );

#if 1
	if (authEdit.size()  < 1)
		return;
#endif

	assert(authEdit.size() >= 1);
	if (authTok) {
		authEdit[0]->clear();
		if (authEdit.size() >= 2)
			authEdit[1]->clear();
		authEdit[0]->setFocus();
	} else {
		authEdit[0]->clear();
		if (loginEdit && loginEdit->isEnabled())
			authEdit[0]->setEnabled( true );
		else {
			setActive( true );
			if (loginEdit && loginEdit->text().isEmpty())
				loginEdit->setFocus();
			else
				authEdit[0]->setFocus();
		}
	}
}

void // virtual
KGenericGreeter::clear()
{
	// assert( !running && !passwd1Edit );
	authEdit[0]->clear();
	if (loginEdit) {
		loginEdit->clear();
		loginEdit->setFocus();
		curUser = QString::null;
	} else
		authEdit[0]->setFocus();
}


// private

void
KGenericGreeter::setActive( bool enable )
{
	if (loginEdit)
		loginEdit->setEnabled( enable );
}

void
KGenericGreeter::setAllActive( bool enable )
{
	for (QList<KLineEdit*>::iterator it = authEdit.begin();
	     it != authEdit.end();
	     ++it)
		(*it)->setEnabled( enable );
}

void
KGenericGreeter::slotLoginLostFocus()
{
	if (!running)
		return;
	if (exp > 0) {
		if (curUser == loginEdit->text())
			return;
		exp = -1;
		handler->gplugReturnText( 0, 0 );
	}
	curUser = loginEdit->text();
	debug( "curUser is %s", qPrintable( curUser ) );
	handler->gplugSetUser( curUser );
}

void
KGenericGreeter::slotActivity()
{
	debug( "slotActivity" );

	if (running)
		handler->gplugActivity();
}

// factory

static bool init( const QString &,
                  QVariant (*getConf)( void *, const char *, const QVariant & ),
                  void *ctx )
{
	echoMode = getConf( ctx, "EchoMode", QVariant( -1 ) ).toInt();
	KGlobal::locale()->insertCatalog( "kgreet_generic" );
	return true;
}

static void done( void )
{
	KGlobal::locale()->removeCatalog( "kgreet_generic" );
	if (log && log != stderr)
		fclose( log );
	log = 0;
}

static KGreeterPlugin *
create( KGreeterPluginHandler *handler,
        QWidget *parent,
        const QString &fixedEntity,
        KGreeterPlugin::Function func,
        KGreeterPlugin::Context ctx )
{
	return new KGenericGreeter( handler, parent, fixedEntity, func, ctx );
}

KDE_EXPORT kgreeterplugin_info kgreeterplugin_info = {
	I18N_NOOP2("@item:inmenu authentication method", "Generic"), "generic",
	kgreeterplugin_info::Local | kgreeterplugin_info::Presettable,
	init, done, create
};

#include "kgreet_generic.moc"
