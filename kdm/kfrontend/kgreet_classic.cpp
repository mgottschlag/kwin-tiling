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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    */

#include "kgreet_classic.h"

#include <klocale.h>
#include <klineedit.h>
#include <kpassdlg.h>
#include <kuser.h>

#include <qlayout.h>
#include <qlabel.h>

class KDMPasswordEdit : public KPasswordEdit {
public:
    KDMPasswordEdit( QWidget *parent ) : KPasswordEdit( parent, 0 ) {}
    KDMPasswordEdit( KPasswordEdit::EchoModes echoMode, QWidget *parent ) : KPasswordEdit( echoMode, parent, 0 ) {}
protected:
    virtual void contextMenuEvent( QContextMenuEvent * ) {}
};

static int echoMode;

KClassicGreeter::KClassicGreeter(
	KGreeterPluginHandler *_handler, QWidget *parent, QWidget *predecessor,
	const QString &_fixedEntity, Function _func, Context _ctx ) :
    QObject(),
    KGreeterPlugin( _handler ),
    fixedUser( _fixedEntity ),
    func( _func ),
    ctx( _ctx ),
    exp( -1 ),
    running( false )
{
    QGridLayout *grid = new QGridLayout( 0, 0, 10 );
    layoutItem = grid;
    QWidget *pred = predecessor;
    int line = 0;

    loginLabel = passwdLabel = passwd1Label = passwd2Label = 0;
    loginEdit = 0;
    passwdEdit = passwd1Edit = passwd2Edit = 0;
    if (ctx == ExUnlock || ctx == ExChangeTok)
	fixedUser = KUser().loginName();
    if (func != ChAuthTok) {
	if (fixedUser.isEmpty()) {
	    loginEdit = new KLineEdit( parent );
	    loginEdit->setContextMenuEnabled( false );
	    loginLabel = new QLabel( loginEdit, i18n("&Username:"), parent );
	    connect( loginEdit, SIGNAL(lostFocus()), SLOT(slotLoginLostFocus()) );
	    if (pred) {
		parent->setTabOrder( pred, loginEdit );
		pred = loginEdit;
	    }
	    grid->addWidget( loginLabel, line, 0 );
	    grid->addWidget( loginEdit, line++, 1 );
	} else if (ctx != Login && ctx != Shutdown) {
	    loginLabel = new QLabel( i18n("Username:"), parent );
	    grid->addWidget( loginLabel, line, 0 );
	    grid->addWidget( new QLabel( fixedUser, parent ), line++, 1 );
	}
	if (echoMode == -1)
	    passwdEdit = new KDMPasswordEdit( parent );
	else
	    passwdEdit = new KDMPasswordEdit( (KPasswordEdit::EchoModes)echoMode, parent );
	passwdLabel = new QLabel( passwdEdit,
	    func == Authenticate ? i18n("&Password:") : i18n("Current &password:"), parent );
	if (pred) {
	    parent->setTabOrder( pred, passwdEdit );
	    pred = passwdEdit;
	}
	grid->addWidget( passwdLabel, line, 0 );
	grid->addWidget( passwdEdit, line++, 1 );
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
	passwd1Label = new QLabel( passwd1Edit, i18n("&New password:"), parent );
	passwd2Label = new QLabel( passwd2Edit, i18n("Con&firm password:"), parent );
	if (pred) {
	    parent->setTabOrder( pred, passwd1Edit );
	    parent->setTabOrder( passwd1Edit, passwd2Edit );
	}
	grid->addWidget( passwd1Label, line, 0 );
	grid->addWidget( passwd1Edit, line++, 1 );
	grid->addWidget( passwd2Label, line, 0 );
	grid->addWidget( passwd2Edit, line, 1 );
    }

    QLayoutIterator it = static_cast<QLayout *>(layoutItem)->iterator();
    for (QLayoutItem *itm = it.current(); itm; itm = ++it)
	 itm->widget()->show();
}

// virtual
KClassicGreeter::~KClassicGreeter()
{
    abort();
    QLayoutIterator it = static_cast<QLayout *>(layoutItem)->iterator();
    for (QLayoutItem *itm = it.current(); itm; itm = ++it)
	 delete itm->widget();
    delete layoutItem;
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
    if (field)
	passwdEdit->setFocus();
    else
	loginEdit->selectAll();
    curUser = entity;
    handler->gplugSetUser( entity );
}

QString // virtual 
KClassicGreeter::getEntity() const
{
    return fixedUser.isEmpty() ? loginEdit->text() : fixedUser;
}

void // virtual
KClassicGreeter::setUser( const QString &user )
{
    // assert (fixedUser.isEmpty());
    curUser = user;
    loginEdit->setText( user );
    passwdEdit->setFocus();
}

void // virtual
KClassicGreeter::setEnabled( bool enable )
{
    // assert (!passwd1Label);
    // assert (func == Authenticate && ctx == Shutdown);
//    if (loginLabel)
//	loginLabel->setEnabled( enable );
    passwdLabel->setEnabled( enable );
    setActive( enable );
    if (enable)
	passwdEdit->setFocus();
}

void // private
KClassicGreeter::returnData()
{
    switch (exp++) {
    case 0:
	handler->gplugReturnText(
	    (loginEdit ? loginEdit->text() : fixedUser).local8Bit(),
	    KGreeterPluginHandler::IsUser );
	break;
    case 1:
	handler->gplugReturnText( passwdEdit->password(),
				  KGreeterPluginHandler::IsPassword );
	break;
    case 2:
	handler->gplugReturnText( passwd1Edit->password(), 0 );
	break;
    default: // case 3:
	handler->gplugReturnText( passwd2Edit->password(),
				  KGreeterPluginHandler::IsPassword );
	break;
    }
}

bool // virtual
KClassicGreeter::textMessage( const char *, bool )
{
    return false;
}

void // virtual
KClassicGreeter::textPrompt( const char *, bool, bool nonBlocking )
{
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
    if (passwdEdit && passwdEdit->isEnabled()) {
	authTok = false;
	if (func == Authenticate || ctx == ChangeTok || ctx == ExChangeTok)
	    exp = -1;
	else
	    exp = 1;
    } else {
	if (running) { // what a hack ... PAM sucks.
	    passwd1Edit->erase();
	    passwd2Edit->erase();
	}
	passwd1Edit->setFocus();
	authTok = true;
	exp = 2;
    }
    has = -1;
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
    if (exp < 0) {
	exp = authTok ? 2 : (ctx == Login || ctx == Shutdown) ? 0 : 1;
	handler->gplugStart();
    } else if (has >= exp)
	returnData();
}

void // virtual
KClassicGreeter::abort()
{
    if (exp >= 0) {
	exp = -1;
	handler->gplugReturnText( 0, 0 );
    }
}

void // virtual
KClassicGreeter::succeeded()
{
    // assert( running );
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
    // assert( running );
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
	setActive( true );
	passwdEdit->erase();
	if (loginEdit && loginEdit->text().isEmpty())
	    loginEdit->setFocus();
	else
	    passwdEdit->setFocus();
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
    if (exp > 0) {
	if (curUser == loginEdit->text())
	    return;
	exp = -1;
	handler->gplugReturnText( 0, 0 );
    }
    curUser = loginEdit->text();
    handler->gplugSetUser( curUser );
}


// factory

static bool init(
	const QString &,
	QVariant (*getConf)( void *, const char *, const QVariant & ),
	void *ctx )
{
    echoMode = getConf( ctx, "EchoMode", QVariant() ).toInt();
    return true;
}

static KGreeterPlugin *
create(
    KGreeterPluginHandler *handler, QWidget *parent, QWidget *predecessor,
    const QString &fixedEntity,
    KGreeterPlugin::Function func,
    KGreeterPlugin::Context ctx )
{
    return new KClassicGreeter( handler, parent, predecessor, fixedEntity, func, ctx );
}

kgreeterplugin_info kgreeterplugin_info = {
    I18N_NOOP("Username + password (classic)"), "classic", kgreeterplugin_info::Local, init, 0, create
};

#include "kgreet_classic.moc"
