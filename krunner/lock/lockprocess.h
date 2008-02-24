//===========================================================================
//
// This file is part of the KDE project
//
// Copyright 1999 Martin R. Jones <mjones@kde.org>
// Copyright 2003 Oswald Buddenhagen <ossi@kde.org>
//

#ifndef LOCKPROCESS_H
#define LOCKPROCESS_H

#include <K3Process>

#include <QWidget>
#include <QTimer>
#include <QStack>
#include <QList>
#include <QMessageBox>
#include <QPixmap>

#include <X11/Xlib.h>
#include <fixx11h.h>

class KLibrary;

struct kgreeterplugin_info;

struct GreeterPluginHandle {
    KLibrary *library;
    kgreeterplugin_info *info;
};

//===========================================================================
//
// Screen saver handling process.  Handles screensaver window,
// starting screensaver hacks, and password entry.
//
class LockProcess
    : public QWidget
{
    Q_OBJECT
public:
    explicit LockProcess(bool child_saver = false, bool useBlankOnly = false);
    ~LockProcess();

    bool lock();

    bool defaultSave();

    bool dontLock();

    void setChildren(QList<int> children) { child_sockets = children; }
    void setParent(int fd) { mParent = fd; }

    void msgBox( QWidget *parent, QMessageBox::Icon type, const QString &txt );
    int execDialog( QDialog* dlg );
    
public Q_SLOTS:
    void quitSaver();
    void preparePopup();
    void cleanupPopup();

protected:
    virtual bool x11Event(XEvent *);
    virtual void timerEvent(QTimerEvent *);

private Q_SLOTS:
    void hackExited();
    void signalPipeSignal();
    bool startLock();
    void suspend();
    void checkDPMSActive();
    void slotDeadTimePassed();

private:
    void configure();
    void readSaver();
    void createSaverWindow();
    void hideSaverWindow();
    void saveVRoot();
    void setVRoot(Window win, Window rw);
    void removeVRoot(Window win);
    bool grabKeyboard();
    bool grabMouse();
    bool grabInput();
    void ungrabInput();
    void cantLock(const QString &reason);
    bool startSaver();
    void stopSaver();
    bool startHack();
    void stopHack();
    void setupSignals();
    bool checkPass();
    void stayOnTop();
    void lockXF86();
    void unlockXF86();
    void resume( bool force );
    static QVariant getConf(void *ctx, const char *key, const QVariant &dflt);

    bool        mLocked;
    int         mLockGrace;
    int         mPriority;
    bool        mBusy;
    K3Process    mHackProc;
    int         mRootWidth;
    int         mRootHeight;
    QString     mSaverExec;
    QString     mSaver;
    bool        mOpenGLVisual;
    bool        child_saver;
    QList<int> child_sockets;
    int         mParent;
    bool        mUseBlankOnly;
    bool        mSuspended;
    QTimer      mSuspendTimer;
    bool        mVisibility;
    bool        mDPMSDepend;
    QTimer      mCheckDPMS;
    QStack< QWidget* > mDialogs;
    bool        mRestoreXF86Lock;
    bool        mForbidden;
    QStringList mPlugins, mPluginOptions;
    QString     mMethod;
    GreeterPluginHandle greetPlugin;
    QPixmap     mSavedScreen;
    int         mAutoLogoutTimerId;
    int         mAutoLogoutTimeout;
    bool        mAutoLogout;
};

#endif

