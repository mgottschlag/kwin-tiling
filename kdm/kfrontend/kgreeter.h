    /*

    Greeter module for xdm
    $Id$

    Copyright (C) 1997, 1998 Steffen Hansen <hansen@kde.org>
    Copyright (C) 2000-2002 Oswald Buddenhagen <ossi@kde.org>


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

#include "kfdialog.h"

#include <kapplication.h>

#include <qvaluevector.h>
#include <qlineedit.h>
#include <qmessagebox.h>

class KdmClock;
class KConsole;
class UserListView;

class KListView;
class KSimpleConfig;
class KPasswordEdit;

class QTimer;
class QLabel;
class QFrame;
class QPushButton;
class QPopupMenu;
class QComboBox;
class QListViewItem;

class GreeterApp : public KApplication {
    typedef KApplication inherited;

public:
    GreeterApp();
    virtual bool x11EventFilter( XEvent * );

protected:
    virtual void timerEvent( QTimerEvent * );

private:
    static void sigAlarm( int );

    int pingInterval;
};


class KLoginLineEdit : public QLineEdit {
    Q_OBJECT
    typedef QLineEdit inherited;

public:
    KLoginLineEdit( QWidget *parent = 0 ) : inherited( parent ) { setMaxLength( 100 ); }

signals:
    void lost_focus();

protected:
    void focusOutEvent( QFocusEvent *e );
};


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


class KGreeter : public FDialog {
    Q_OBJECT
    typedef FDialog inherited;

public:
    KGreeter();
    ~KGreeter();
    void UpdateLock();
 
public slots:
    void accept();
    void reject();
    void chooser_button_clicked();
    void console_button_clicked();
    void quit_button_clicked();
    void shutdown_button_clicked();
    void slot_user_name( QListViewItem * );
    void slot_user_doubleclicked();
    void slot_session_selected( int );
    void SetTimer();
    void timerDone();
    void sel_user();
    void load_wm();
    void slotActivateMenu( int id );

protected:
    void timerEvent( QTimerEvent * ) {};
    void keyPressEvent( QKeyEvent * );
    void keyReleaseEvent( QKeyEvent * );

private:
    void set_wm( int );
    void insertUsers( UserListView * );
    void insertUser( UserListView *, const QImage &, const QString &, struct passwd * );
    void putSession(const QString &, const QString &, bool, const char *);
    void insertSessions();
#define errorbox QMessageBox::Critical
#define sorrybox QMessageBox::Warning
#define infobox QMessageBox::Information
    void MsgBox( QMessageBox::Icon typ, QString msg ) { KFMsgBox::box( this, typ, msg ); }
    void Inserten( const QString& txt, const char *member );
    void Inserten( const QString& txt, QPopupMenu *cmnu );
    bool verifyUser( bool );
    void updateStatus();

    QString		enam, user_pic_dir;
    KSimpleConfig	*stsfile;
    UserListView	*user_view;
    KdmClock		*clock;
    QLabel		*pixLabel;
    QLabel		*loginLabel, *passwdLabel;
    KLoginLineEdit	*loginEdit;
    KPasswordEdit	*passwdEdit; 
    QFrame		*separator;
    QTimer		*timer;
    QLabel		*failedLabel;
    QPushButton		*goButton, *clearButton;
    QPushButton		*menuButton;
    QPopupMenu		*optMenu, *sessMenu;
    QPushButton		*quitButton;
    QPushButton		*shutdownButton;
    QValueVector<SessType> sessionTypes;
    int			nnormals, nspecials;
    int			curprev, cursel;
    bool		prevvalid;
    int			capslocked;
    bool		loginfailed;
#ifdef BUILTIN_XCONSOLE
    KConsole		*consoleView;
#endif
};


#endif /* KGREETER_H */

/*
 * Local variables:
 * mode: c++
 * c-file-style: "k&r"
 * End:
 */
