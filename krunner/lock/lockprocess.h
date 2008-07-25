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
#include <QMessageBox>
#include <QPixmap>

#include <X11/Xlib.h>
#include <fixx11h.h>

class QDBusInterface;
class KLibrary;

struct KGreeterPluginInfo;

struct GreeterPluginHandle {
    KLibrary *library;
    KGreeterPluginInfo *info;
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
    //dbus methods
    /**
     * bring up the password dialog with @param reason displayed instead of the usual "this session
     * is locked" message.
     * @return true if the password was entered correctly
     * if this returns true, there is a grace period where the screensaver can be freely unlocked
     * with the unlock method without re-entering the password.
     */
    Q_SCRIPTABLE bool checkPass(const QString &reason);
    /**
     * this will unlock and quit the screensaver, asking for a password first if necessary
     */
    Q_SCRIPTABLE void unlock();
    /**
     * immediately end the "free unlock" grace period; if the screen is locked, it will now require
     * a password to unlock.
     * this has no effect if the screen wasn't locked in the first place.
     */
    Q_SCRIPTABLE void endFreeUnlock();

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
    /**
     * a new dbus service has come in
     */
    void newService(QString name);
    /**
     * set the winid of plasma's view
     * so that we can control it
     */
    void setPlasmaView(uint id); //it's really a WId but qdbuscpp2xml is dumb
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
     * if the password dialog is not suppressed, this execs it
     * if the dialog *is* suppressed we just restart the timer
     * @return true iff the password was checked and is valid
     */
    bool checkPass();
    void stayOnTop();
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

    bool        mLocked;
    int         mLockGrace;
    int         mPriority;
    bool        mBusy;
    KProcess    mHackProc;
    KProcess    mPlasmaProc;
    QDBusInterface *mPlasmaDBus;
    WId         mPlasmaView;
    bool        mPlasmaEnabled;
    bool        mFreeUnlock;
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
    QTimer      mSuppressUnlock;
    QList<WId>  mForeignWindows;
    QList<WId>  mForeignInputWindows;
};

#endif

