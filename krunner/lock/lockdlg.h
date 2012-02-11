//===========================================================================
//
// This file is part of the KDE project
//
// Copyright 1999 Martin R. Jones <mjones@kde.org>
// Copyright 2003 Oswald Buddenhagen <ossi@kde.org>
//

#ifndef __LOCKDLG_H__
#define __LOCKDLG_H__

#include <kgreeterplugin.h>

#include <KDialog>

#include <QLabel>
#include <QTimerEvent>
#include <QFrame>
#include <QGridLayout>
#include <QEvent>

struct GreeterPluginHandle;
class LockProcess;
class QFrame;
class QGridLayout;
class QLabel;
class KPushButton;
class QSocketNotifier;
class QTreeWidget;

//===========================================================================
//
// Simple dialog for entering a password.
// It does not handle password validation.
//
class PasswordDlg : public KDialog, public KGreeterPluginHandler
{
    Q_OBJECT

public:
    PasswordDlg(LockProcess *parent, GreeterPluginHandle *plugin, const QString &text = QString());
    ~PasswordDlg();
    virtual void setVisible(bool visible);

    // from KGreetPluginHandler
    virtual void gplugReturnText( const char *text, int tag );
    virtual void gplugReturnBinary( const char *data );
    virtual void gplugSetUser( const QString & );
    virtual void gplugStart();
    virtual void gplugChanged();
    virtual void gplugActivity();
    virtual void gplugMsgBox( QMessageBox::Icon type, const QString &text );
    virtual bool gplugHasNode( const QString &id );

protected:
    virtual void timerEvent(QTimerEvent *);
    virtual bool eventFilter(QObject *, QEvent *);

private Q_SLOTS:
    void slotSwitchUser();
    void slotSessionActivated();
    void slotStartNewSession();
    void slotOK();
    void slotActivity();
    void handleVerify();

private:
    void capsLocked();
    void updateLabel();
    int Reader (void *buf, int count);
    bool GRead (void *buf, int count);
    bool GWrite (const void *buf, int count);
    bool GSendInt (int val);
    bool GSendStr (const char *buf);
    bool GSendArr (int len, const char *buf);
    bool GRecvInt (int *val);
    bool GRecvArr (char **buf);
    void reapVerify();
    void cantCheck();
    GreeterPluginHandle *mPlugin;
    KGreeterPlugin *greet;
    QFrame      *frame;
    QGridLayout *frameLayout;
    QLabel      *mStatusLabel;
    KPushButton *mNewSessButton, *ok, *cancel;
    int         mFailedTimerId;
    int         mTimeoutTimerId;
    int         mCapsLocked;
    bool        mUnlockingFailed;
    int         sPid, sFd;
    QSocketNotifier *sNot;
    QTreeWidget *lv;
};

#endif

