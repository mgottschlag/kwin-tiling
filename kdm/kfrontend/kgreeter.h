    /*

    Greeter widget for kdm

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


#ifndef KGREETER_H
#define KGREETER_H

#include "kgverify.h"
#include "kgdialog.h"

class KdmClock;
class UserListView;

class KListView;
class KSimpleConfig;

class QLabel;
class QPushButton;
class QPopupMenu;
class QListViewItem;

struct SessType {
    QString name, type;
    bool hid;
    int prio;

    SessType() {}
    SessType( const QString &n, const QString &t, bool h, int p ) :
	name( n ), type( t ), hid( h ), prio( p ) {}
    bool operator<( const SessType &st ) {
	return hid != st.hid ? hid < st.hid :
		prio != st.prio ? prio < st.prio :
		 name < st.name;
    }
};

class KGreeter : public KGDialog, public KGVerifyHandler {
    Q_OBJECT
    typedef KGDialog inherited;

public:
    KGreeter();
    ~KGreeter();
 
public slots:
    void accept();
    void reject();
    void slotUserClicked( QListViewItem * );
    void slotSessionSelected( int );
    void slotUserEntered();
    void slotExit();
    void slotShutdown();
    void slotConsole();

private:
    void insertUser( const QImage &, const QString &, struct passwd * );
    void insertUsers();
    void putSession( const QString &, const QString &, bool, const char * );
    void insertSessions();
    void pluginSetup();
    void setPrevWM( int );

    QString		curUser, dName;
    KSimpleConfig	*stsFile;
    UserListView	*userView;
    QStringList		*userList;
    KdmClock		*clock;
    QLabel		*pixLabel;
    KGVerify    	*verify;
    QPushButton		*goButton;
    QPopupMenu		*sessMenu;
    QValueVector<SessType> sessionTypes;
    int			nNormals, nSpecials;
    int			curPrev, curSel;
    bool		prevValid;
    bool		needLoad;

    static int		curPlugin;
    static PluginList	pluginList;

private slots:
    void slotLoadPrevWM();

public: // from KGVerifyHandler
    virtual void verifyPluginChanged( int id );
    virtual void verifyOk();
    virtual void verifyFailed();
    virtual void verifyRetry();
    virtual void verifySetUser( const QString &user );
};


#endif /* KGREETER_H */
