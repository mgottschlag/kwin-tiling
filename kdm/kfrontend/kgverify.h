    /*

    Shell for kdm conversation plugins

    Copyright (C) 1997, 1998 Steffen Hansen <hansen@kde.org>
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


#ifndef KGVERIFY_H
#define KGVERIFY_H

#include "kgreeterplugin.h"
#include "kfdialog.h"

#include <qlayout.h>
#include <qtimer.h>
#include <qvaluevector.h>

#include <sys/time.h>
#include <time.h>

// helper class, nuke when qt supports suspend()/resume()
class QXTimer : public QObject {
    Q_OBJECT
    typedef QObject inherited;

public:
    QXTimer();
    void start( int msec );
    void stop();
    void suspend();
    void resume();

signals:
    void timeout();

private slots:
    void slotTimeout();

private:
    QTimer timer;
    struct timeval stv;
    long left;
};

class KGVerifyHandler {
public:
    virtual void verifyPluginChanged( int id ) = 0;
    virtual void verifyOk() = 0;
    virtual void verifyFailed() = 0;
    virtual void verifyRetry() = 0;
    virtual void verifySetUser( const QString &user ) = 0;
};

class QWidget;
class QLabel;
class QPopupMenu;
class QTimer;
class KPushButton;
class KLibrary;

struct GreeterPluginHandle {
    KLibrary *library;
    kgreeterplugin_info *info;
};

typedef QValueVector<int> PluginList;

class KGVerify : public QObject, public KGreeterPluginHandler {
    Q_OBJECT
    typedef QObject inherited;

public:
    KGVerify( KGVerifyHandler *handler, QWidget *parent,
	      QWidget *predecessor, const QString &fixedEntity,
	      const PluginList &pluginList,
	      KGreeterPlugin::Function func, KGreeterPlugin::Context ctx );
    virtual ~KGVerify();
    QLayout *getLayout() const { return grid; }
    QPopupMenu *getPlugMenu();
    void loadUsers( const QStringList &users );
    void presetEntity( const QString &entity, int field );
    QString getEntity() const;
    void setUser( const QString &user );
    void selectPlugin( int id );
    bool entitiesLocal() const;
    bool entitiesFielded() const;
    QString pluginName() const;
    void setEnabled( bool on );
    void abort();
    void suspend();
    void resume();
    void accept();
    void reject();

    int coreLock;

    static bool handleFailVerify( QWidget *parent );
    static PluginList init( const QStringList &plugins );
    static void done();

public slots:
    void start();

protected:
    bool eventFilter( QObject *, QEvent * );

private:
    void MsgBox( QMessageBox::Icon typ, const QString &msg );
    void setTimer();
    void updateLockStatus();
    void updateStatus();
    void handleVerify();

    QXTimer		timer;
    QString		fixedEntity, curUser;
    PluginList		pluginList;
    KGVerifyHandler	*handler;
    QWidget		*parent, *predecessor;
    QGridLayout		*grid;
    KGreeterPlugin	*greet;
    QLabel		*failedLabel;
    QPopupMenu		*plugMenu;
    int			curPlugin;
    int			failedLabelState;
    KGreeterPlugin::Function func;
    KGreeterPlugin::Context ctx;
    bool		capsLocked;
    bool		enabled, running, suspended, failed, delayed, cont;
    bool		authTok, hasBegun;

    static void VMsgBox( QWidget *parent, const QString &user, QMessageBox::Icon type, const QString &mesg );
    static void VErrBox( QWidget *parent, const QString &user, const char *msg );
    static void VInfoBox( QWidget *parent, const QString &user, const char *msg );

    static QValueVector<GreeterPluginHandle> greetPlugins;

private slots:
    void slotPluginSelected( int id );
    void slotTimeout();

public: // from KGreetPluginHandler
    virtual void gplugReturnText( const char *text, int tag );
    virtual void gplugReturnBinary( const char *data );
    virtual void gplugSetUser( const QString &user );
    virtual void gplugStart();
    virtual void gplugActivity();
    virtual void gplugMsgBox( QMessageBox::Icon type, const QString &text );

    static QVariant getConf( void *ctx, const char *key, const QVariant &dflt );
};

class KGChTok : public FDialog, public KGVerifyHandler {
    Q_OBJECT
    typedef FDialog inherited;

public:
    KGChTok( QWidget *parent, const QString &user,
	     const PluginList &pluginList, int curPlugin,
	     KGreeterPlugin::Function func, KGreeterPlugin::Context ctx );
    ~KGChTok();

public slots:
    void accept();

private:
    KPushButton		*okButton, *cancelButton;
    KGVerify		*verify;

public: // from KGVerifyHandler
    virtual void verifyPluginChanged( int id );
    virtual void verifyOk();
    virtual void verifyFailed();
    virtual void verifyRetry();
    virtual void verifySetUser( const QString &user );
};


#endif /* KGVERIFY_H */
