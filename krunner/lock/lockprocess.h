//===========================================================================
//
// This file is part of the KDE project
//
// Copyright (c) 1999 Martin R. Jones <mjones@kde.org>
//

#ifndef __LOCKENG_H__
#define __LOCKENG_H__

#include <qwidget.h>
#include <kprocess.h>
#include <qtimer.h>
#include <qvaluestack.h>

#include <X11/Xlib.h>

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

    void lock();

    void defaultSave();

    void dontLock();

    void setChildren(QValueList<int> children) { child_sockets = children; }
    void setParent(int fd) { mParent = fd; }

    void registerDialog( QWidget* w );
    void unregisterDialog( QWidget* w );
    
public slots:
    void quitSaver();

protected:
    virtual bool x11Event(XEvent *);

private slots:
    void hackExited(KProcess *);
    void slotStart();
    void sigtermPipeSignal();
    void startNewSession();
    void actuallySetLock();
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
    void startSaver();
    void stopSaver();
    bool startHack();
    void stopHack();
    void setupSignals();
    bool checkPass();
    void stayOnTop();
    void lockXF86();
    void unlockXF86();

    bool        mLock;
    int         mPriority;
    bool        mLockOnce;
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
    bool	mUseBlankOnly;
    bool        mSuspended;
    QTimer      mSuspendTimer;
    bool        mVisibility;
    QTimer      mCheckDPMS;
    QValueStack< QWidget* > mDialogs;
    bool        mRestoreXF86Lock;
    bool	mForbidden;
};

#endif

