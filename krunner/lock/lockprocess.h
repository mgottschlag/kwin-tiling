//===========================================================================
//
// This file is part of the KDE project
//
// Copyright (c) 1999 Martin R. Jones <mjones@kde.org>
// Copyright (c) 2003 Oswald Buddenhagen <ossi@kde.org>
//

#ifndef __LOCKENG_H__
#define __LOCKENG_H__

#include <kgreeterplugin.h>

#include <kprocess.h>

#include <qwidget.h>
#include <qtimer.h>
#include <qvaluestack.h>
#include <qmessagebox.h>
#include <qpixmap.h>

#include <X11/Xlib.h>

class KLibrary;

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
    LockProcess(bool child_saver = false, bool useBlankOnly = false);
    ~LockProcess();

    bool lock();

    bool defaultSave();

    bool dontLock();

    void setChildren(QValueList<int> children) { child_sockets = children; }
    void setParent(int fd) { mParent = fd; }

    void msgBox( QMessageBox::Icon type, const QString &txt );
    int execDialog( QDialog* dlg );
    
public slots:
    void quitSaver();

protected:
    virtual bool x11Event(XEvent *);

private slots:
    void hackExited(KProcess *);
    void sigtermPipeSignal();
    void startNewSession();
    bool startLock();
    void suspend();
    void resume();
    void checkDPMSActive();

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
    void xdmFifoCmd(const char *cmd);
    void xdmFifoLockCmd(const char *cmd);
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
    static QVariant getConf(void *ctx, const char *key, const QVariant &dflt);

    bool        mLocked;
    int         mLockGrace;
    int         mPriority;
    bool        mBusy;
    Colormap    mColorMap;
    KProcess    mHackProc;
    int         mRootWidth;
    int         mRootHeight;
    QString     mSaverExec;
    QString     mSaver;
    QString     mXdmFifoName;
    bool        child_saver;
    QValueList<int> child_sockets;
    int         mParent;
    bool        mUseBlankOnly;
    bool        mSuspended;
    QTimer      mSuspendTimer;
    bool        mVisibility;
    bool        mDPMSDepend;
    QTimer      mCheckDPMS;
    QValueStack< QWidget* > mDialogs;
    bool        mRestoreXF86Lock;
    bool        mForbidden;
    QStringList mPlugins, mPluginOptions;
    QString     mMethod;
    GreeterPluginHandle greetPlugin;
    QPixmap     mSavedScreen;
};

#endif

