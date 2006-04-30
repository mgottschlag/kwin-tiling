/*

Conversation widget for kdm greeter

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

#include "kgreet_classic.h"
#include "themer/kdmthemer.h"
#include "themer/kdmitem.h"

#include <klocale.h>
#include <klineedit.h>
#include <kpassworddialog.h>
#include <kuser.h>

#include <qregexp.h>
#include <qlayout.h>
#include <qlabel.h>

class KDMPasswordEdit : public KPasswordEdit {
public:
	KDMPasswordEdit( QWidget *parent ) : KPasswordEdit( parent ) {}
	KDMPasswordEdit( KPasswordEdit::EchoModes echoMode, QWidget *parent ) : KPasswordEdit( echoMode, parent ) {}
protected:
	virtual void contextMenuEvent( QContextMenuEvent * ) {}
};

static int echoMode;

KClassicGreeter::KClassicGreeter( KGreeterPluginHandler *_handler,
                                  KdmThemer *themer,
                                  QWidget *parent, QWidget *pred,
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
	KdmItem *user_entry = 0, *pw_entry = 0;
	QGridLayout *grid = 0;
	int line = 0;

	layoutItem = 0;

	if (themer &&
	    (!(user_entry = themer->findNode( "user-entry" )) ||
	     !(pw_entry = themer->findNode( "pw-entry" ))))
		themer = 0;

	if (!themer)
		layoutItem = grid = new QGridLayout( 0, 0, 10 );

	loginLabel = passwdLabel = passwd1Label = passwd2Label = 0;
	loginEdit = 0;
	passwdEdit = passwd1Edit = passwd2Edit = 0;
	if (ctx == ExUnlock || ctx == ExChangeTok)
		fixedUser = KUser().loginName();
	if (func != ChAuthTok) {
		if (fixedUser.isEmpty()) {
			loginEdit = new KLineEdit( parent );
			loginEdit->setContextMenuEnabled( false );
			connect( loginEdit, SIGNAL(lostFocus()), SLOT(slotLoginLostFocus()) );
			connect( loginEdit, SIGNAL(lostFocus()), SLOT(slotActivity()) );
			connect( loginEdit, SIGNAL(textChanged( const QString & )), SLOT(slotActivity()) );
			connect( loginEdit, SIGNAL(selectionChanged()), SLOT(slotActivity()) );
			if (pred) {
				parent->setTabOrder( pred, loginEdit );
				pred = loginEdit;
			}
			if (!grid) {
				loginEdit->adjustSize();
				user_entry->setWidget( loginEdit );
			} else {
				loginLabel = new QLabel( i18n("&Username:"), parent );
				loginLabel->setBuddy( loginEdit );
				grid->addWidget( loginLabel, line, 0 );
				grid->addWidget( loginEdit, line++, 1 );
			}
		} else if (ctx != Login && ctx != Shutdown && grid) {
			loginLabel = new QLabel( i18n("Username:"), parent );
			grid->addWidget( loginLabel, line, 0 );
			grid->addWidget( new QLabel( fixedUser, parent ), line++, 1 );
		}
		if (echoMode == -1)
			passwdEdit = new KDMPasswordEdit( parent );
		else
			passwdEdit = new KDMPasswordEdit( (KPasswordEdit::EchoModes)echoMode,
			                                  parent );
		connect( passwdEdit, SIGNAL(textChanged( const QString & )),
		         SLOT(slotActivity()) );
		connect( passwdEdit, SIGNAL(lostFocus()), SLOT(slotActivity()) );
		if (pred) {
			parent->setTabOrder( pred, passwdEdit );
			pred = passwdEdit;
		}
		if (!grid) {
			passwdEdit->adjustSize();
			pw_entry->setWidget( passwdEdit );
		} else {
                        passwdLabel = new QLabel( func == Authenticate ?
			                            i18n("&Password:") :
			                            i18n("Current &password:"),
			                          parent );
                        passwdLabel->setBuddy( passwdEdit );
			grid->addWidget( passwdLabel, line, 0 );
			grid->addWidget( passwdEdit, line++, 1 );
		}
		if (loginEdit)
			loginEdit->setFocus();
		else
			passwdEdit->setFocus();
	}
	if (func != Authenticate) {
		if (echoMode == -1) {
			passwd1Edit = new KDMPasswordEdit( (KPasswordEdit::EchoModes)echoMode, parent );
			passwd2Edit = new KDMPasswordEdit( (KPasswordEdit::EchoModes)echoMode, parent );
		} else {
			passwd1Edit = new KDMPasswordEdit( parent );
			passwd2Edit = new KDMPasswordEdit( parent );
		}
		passwd1Label = new QLabel( i18n("&New password:"), parent );
		passwd1Label->setBuddy( passwd1Edit );
		passwd2Label = new QLabel( i18n("Con&firm password:"), parent );
		passwd2Label->setBuddy( passwd2Edit );
		if (pred) {
			parent->setTabOrder( pred, passwd1Edit );
			parent->setTabOrder( passwd1Edit, passwd2Edit );
		}
		if (grid) {
			grid->addWidget( passwd1Label, line, 0 );
			grid->addWidget( passwd1Edit, line++, 1 );
			grid->addWidget( passwd2Label, line, 0 );
			grid->addWidget( passwd2Edit, line, 1 );
		}
		if (!passwdEdit)
			passwd1Edit->setFocus();
	}
}

// virtual
KClassicGreeter::~KClassicGreeter()
{
	abort();
	if (!layoutItem) {
		delete loginEdit;
		delete passwdEdit;
		return;
	}
}

void // virtual
KClassicGreeter::loadUsers( const QStringList &users )
{
	KCompletion *userNamesCompletion = new KCompletion;
	userNamesCompletion->setItems( users );
	loginEdit->setCompletionObject( userNamesCompletion );
	loginEdit->setAutoDeleteCompletionObject( true );
	loginEdit->setCompletionMode( KGlobalSettings::CompletionAuto );
}

void // virtual
KClassicGreeter::presetEntity( const QString &entity, int field )
{
	loginEdit->setText( entity );
	if (field == 1)
		passwdEdit->setFocus();
	else {
		loginEdit->setFocus();
		loginEdit->selectAll();
		if (field == -1) {
			passwdEdit->setText( "     " );
			passwdEdit->setEnabled( false );
			authTok = false;
		}
	}
	curUser = entity;
}

QString // virtual
KClassicGreeter::getEntity() const
{
	return fixedUser.isEmpty() ? loginEdit->text() : fixedUser;
}

void // virtual
KClassicGreeter::setUser( const QString &user )
{
	// assert( fixedUser.isEmpty() );
	curUser = user;
	loginEdit->setText( user );
	passwdEdit->setFocus();
}

void // virtual
KClassicGreeter::setEnabled( bool enable )
{
	// assert( !passwd1Label );
	// assert( func == Authenticate && ctx == Shutdown );
//	if (loginLabel)
//		loginLabel->setEnabled( enable );
	passwdLabel->setEnabled( enable );
	setActive( enable );
	if (enable)
		passwdEdit->setFocus();
}

void // private
KClassicGreeter::returnData()
{
	switch (exp) {
	case 0:
		handler->gplugReturnText( (loginEdit ? loginEdit->text() :
		                                       fixedUser).toLocal8Bit(),
		                          KGreeterPluginHandler::IsUser );
		break;
	case 1:
		Q_ASSERT(passwdEdit);
		handler->gplugReturnText( passwdEdit ? passwdEdit->password() : 0,
		                          KGreeterPluginHandler::IsPassword |
		                          KGreeterPluginHandler::IsSecret );
		break;
	case 2:
		Q_ASSERT(passwd1Edit);
		handler->gplugReturnText( passwd1Edit ? passwd1Edit->password() : 0,
		                          KGreeterPluginHandler::IsSecret );
		break;
	default: // case 3:
		Q_ASSERT(passwd2Edit);
		handler->gplugReturnText( passwd2Edit ? passwd2Edit->password() : 0,
		                          KGreeterPluginHandler::IsNewPassword |
		                          KGreeterPluginHandler::IsSecret );
		break;
	}
}

bool // virtual
KClassicGreeter::textMessage( const char *text, bool err )
{
	if (!err &&
	    QString( text ).indexOf( QRegExp( "^Changing password for [^ ]+$" ) ) >= 0)
		return true;
	return false;
}

void // virtual
KClassicGreeter::textPrompt( const char *prompt, bool echo, bool nonBlocking )
{
	pExp = exp;
	if (echo)
		exp = 0;
	else if (!authTok)
		exp = 1;
	else {
		QString pr( prompt );
		if (pr.indexOf( QRegExp( "\\bpassword\\b", Qt::CaseInsensitive ) ) >= 0) {
			if (pr.indexOf( QRegExp( "\\b(re-?(enter|type)|again|confirm)\\b",
			                      Qt::CaseInsensitive ) ) >= 0)
				exp = 3;
			else if (pr.indexOf( QRegExp( "\\bnew\\b", Qt::CaseInsensitive ) ) >= 0)
				exp = 2;
			else { // QRegExp( "\\b(old|current)\\b", Qt::CaseInsensitive ) is too strict
				handler->gplugReturnText( "",
				                          KGreeterPluginHandler::IsOldPassword |
				                          KGreeterPluginHandler::IsSecret );
				return;
			}
		} else {
			handler->gplugMsgBox( QMessageBox::Critical,
			                      i18n("Unrecognized prompt \"%1\"",
			                        prompt ) );
			handler->gplugReturnText( 0, 0 );
			exp = -1;
			return;
		}
	}

	if (pExp >= 0 && pExp >= exp) {
		revive();
		has = -1;
	}

	if (has >= exp || nonBlocking)
		returnData();
}

bool // virtual
KClassicGreeter::binaryPrompt( const char *, bool )
{
	// this simply cannot happen ... :}
	return true;
}

void // virtual
KClassicGreeter::start()
{
	authTok = !(passwdEdit && passwdEdit->isEnabled());
	exp = has = -1;
	running = true;
}

void // virtual
KClassicGreeter::suspend()
{
}

void // virtual
KClassicGreeter::resume()
{
}

void // virtual
KClassicGreeter::next()
{
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
	else if (has >= exp)
		returnData();
}

void // virtual
KClassicGreeter::abort()
{
	running = false;
	if (exp >= 0) {
		exp = -1;
		handler->gplugReturnText( 0, 0 );
	}
}

void // virtual
KClassicGreeter::succeeded()
{
	// assert( running || timed_login );
	if (!authTok) {
		setActive( false );
		if (passwd1Edit) {
			authTok = true;
			return;
		}
	} else
		setActive2( false );
	exp = -1;
	running = false;
}

void // virtual
KClassicGreeter::failed()
{
	// assert( running || timed_login );
	setActive( false );
	setActive2( false );
	exp = -1;
	running = false;
}

void // virtual
KClassicGreeter::revive()
{
	// assert( !running );
	setActive2( true );
	if (authTok) {
		passwd1Edit->erase();
		passwd2Edit->erase();
		passwd1Edit->setFocus();
	} else {
		passwdEdit->erase();
		if (loginEdit && loginEdit->isEnabled())
			passwdEdit->setEnabled( true );
		else {
			setActive( true );
			if (loginEdit && loginEdit->text().isEmpty())
				loginEdit->setFocus();
			else
				passwdEdit->setFocus();
		}
	}
}

void // virtual
KClassicGreeter::clear()
{
	// assert( !running && !passwd1Edit );
	passwdEdit->erase();
	if (loginEdit) {
		loginEdit->clear();
		loginEdit->setFocus();
		curUser.clear();
	} else
		passwdEdit->setFocus();
}


// private

void
KClassicGreeter::setActive( bool enable )
{
	if (loginEdit)
		loginEdit->setEnabled( enable );
	if (passwdEdit)
		passwdEdit->setEnabled( enable );
}

void
KClassicGreeter::setActive2( bool enable )
{
	if (passwd1Edit) {
		passwd1Edit->setEnabled( enable );
		passwd2Edit->setEnabled( enable );
	}
}

void
KClassicGreeter::slotLoginLostFocus()
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
	handler->gplugSetUser( curUser );
}

void
KClassicGreeter::slotActivity()
{
	if (running)
		handler->gplugActivity();
}

// factory

static bool init( const QString &,
                  QVariant (*getConf)( void *, const char *, const QVariant & ),
                  void *ctx )
{
	echoMode = getConf( ctx, "EchoMode", QVariant( -1 ) ).toInt();
	KGlobal::locale()->insertCatalog( "kgreet_classic" );
	return true;
}

static void done( void )
{
	KGlobal::locale()->removeCatalog( "kgreet_classic" );
}

static KGreeterPlugin *
create( KGreeterPluginHandler *handler, KdmThemer *themer,
        QWidget *parent, QWidget *predecessor,
        const QString &fixedEntity,
        KGreeterPlugin::Function func,
        KGreeterPlugin::Context ctx )
{
	return new KClassicGreeter( handler, themer, parent, predecessor, fixedEntity, func, ctx );
}

KDE_EXPORT kgreeterplugin_info kgreeterplugin_info = {
	I18N_NOOP("Username + password (classic)"), "classic",
	kgreeterplugin_info::Local | kgreeterplugin_info::Presettable,
	init, done, create
};

#include "kgreet_classic.moc"
