//===========================================================================
//
// This file is part of the KDE project
//
// Copyright 1999 Martin R. Jones <mjones@kde.org>
// Copyright 2003 Oswald Buddenhagen <ossi@kde.org>
// Copyright 2008 Chani Armitage <chanika@gmail.com>
//

#ifndef LOCKPROCESS_H
#define LOCKPROCESS_H

#include <KProcess>

#include <QWidget>
#include <QTimer>
#include <QStack>
#include <QList>
#include <QHash>
#include <QMessageBox>
#include <QPixmap>

#include <X11/Xlib.h>
#include <fixx11h.h>

#include "plasmaapp_interface.h"

class KLibrary;

struct KGreeterPluginInfo;

struct GreeterPluginHandle {
    KLibrary *library;
    KGreeterPluginInfo *info;
};

const int TIMEOUT_CODE = 2; //from PasswordDlg

class QDBusServiceWatcher;

//===========================================================================
//
// Screen saver handling process.  Handles screensaver window,
// starting screensaver hacks, and password entry.
//
class LockProcess
    : public QWidget
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.screenlocker.LockProcess")
public:
    explicit LockProcess(bool child_saver = false, bool useBlankOnly = false);
    ~LockProcess();

    /**
     * start the screensaver locked
     */
    bool lock(bool initial = false);

    /**
     * start the screensaver unlocked
     */
    bool defaultSave();

    /**
     * start the screensaver in plasma setup mode
     * if plasma is disabled this just acts like defaultSave
     */
    bool startSetup();

    /**
     * start the screensaver unlocked, and *never* automatically lock it
     */
    bool dontLock();

    void setChildren(QList<int> children) { child_sockets = children; }
    void setParent(int fd) { mParent = fd; }

    void msgBox( QWidget *parent, QMessageBox::Icon type, const QString &txt );
    int execDialog( QDialog* dlg );

public Q_SLOTS:
    void quitSaver();
    //dbus methods
    /**
     * bring up the password dialog with @param reason displayed instead of the usual "this session
     * is locked" message.
     * @return true if the password was entered correctly
     * if this returns true, it will also unlock the screensaver without quitting.
     * it will re-lock after the lock timeout in the settings
     */
    Q_SCRIPTABLE bool checkPass(const QString &reason);
    /**
     * this will unlock and quit the screensaver, asking for a password first if necessary
     */
    Q_SCRIPTABLE void quit();
    /**
     * immediately lock the screen; it will now require a password to unlock.
     */
    Q_SCRIPTABLE bool startLock();

protected:
    virtual bool x11Event(XEvent *);
    virtual void timerEvent(QTimerEvent *);
    virtual bool eventFilter(QObject *o, QEvent *e);

private Q_SLOTS:
    void hackExited();
    void signalPipeSignal();
    void suspend();
    void checkDPMSActive();
    void slotDeadTimePassed();
    /**
     * check that plasma started properly (used for timeout)
     * and disable it if it failed
     */
    void checkPlasma();
    /**
     * a new dbus service has come in
     */
    void newService(QString name, QString oldOwner, QString newOwner);
    /**
     * tell plasma we're in idle mode
     */
    void deactivatePlasma();
    void lockPlasma();
    /**
     * immediately un-suppress the password dialog
     * FIXME need a better name
     */
    void unSuppressUnlock();

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
    bool startPlasma();
    void stopPlasma();
    void setupSignals();
    /**
     * exec the password dialog
     * @return true iff the password was checked and is valid
     */
    bool checkPass();
    /**
     * returns true if plasma is up and the dbus interface is valid
     */
    bool isPlasmaValid();
    /**
     * give up on plasma, probably because it crashed.
     * this does *not* tell plasma to quit. it just stops using it.
     */
    void disablePlasma();
    /**
     * give a fakefocusin to the right window
     */
    void updateFocus();
    void stayOnTop();
    int findWindowInfo( Window window ); // returns index in windowInfo or -1
    void lockXF86();
    void unlockXF86();
    void resume( bool force );
    enum WindowType { IgnoreWindow = 0 /** regular window to be left below the saver */,
                      SimpleWindow = 1 /** simple popup that can't handle direct input */,
                      InputWindow = 2  /** annoying dialog that needs direct input */,
                      DefaultWindow = 6/** input window that's also the plasma view */
    };
    /**
     * @return the type of window, based on its X property
     */
    WindowType windowType(WId id);

    static QVariant getConf(void *ctx, const char *key, const QVariant &dflt);
    bool loadGreetPlugin();

    bool        mInitialLock;
    bool        mLocked;
    int         mLockGrace;
    int         mPriority;
    bool        mBusy;
    KProcess    mHackProc;
    org::kde::plasmaoverlay::App *mPlasmaDBus;
    QDBusServiceWatcher *mServiceWatcher;
    bool        mPlasmaEnabled;
    bool        mSetupMode;
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
    QHash< QWidget*, QWidget* > mFrames;
    QList<WId>  mVisibleDialogs;
    QQueue<XEvent> mEventQueue;
    bool        mEventRecursed;
    bool        mRestoreXF86Lock;
    bool        mForbidden;
    QStringList mPlugins, mPluginOptions;
    QString     mMethod;
    GreeterPluginHandle greetPlugin;
    QPixmap     mSavedScreen;
    QTimer      mSnapshotTimer;
    int         mAutoLogoutTimerId;
    int         mAutoLogoutTimeout;
    QTimer      mSuppressUnlock;
    int         mSuppressUnlockTimeout;
    QList<WId>  mForeignWindows;
    QList<WId>  mForeignInputWindows;
    struct WindowInfo
    {
        Window window;
        bool viewable;
    };
    QList<WindowInfo> windowInfo;
};

#endif

