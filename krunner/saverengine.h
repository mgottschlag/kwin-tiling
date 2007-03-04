//===========================================================================
//
// This file is part of the KDE project
//
// Copyright (c) 1999 Martin R. Jones <mjones@kde.org>
//

#ifndef __SAVERENGINE_H__
#define __SAVERENGINE_H__

#include <QWidget>
#include <kprocess.h>
#include <QVector>
#include <QDBusConnection>

#include "xautolock.h"
#include "xautolock_c.h"

//===========================================================================
/**
 * Screen saver engine.  Handles screensaver window, starting screensaver
 * hacks, and password entry.
 */
class SaverEngine : public QWidget
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.ScreenSaver")

public:
    SaverEngine();
    ~SaverEngine();

public Q_SLOTS:
    /**
     * Lock the screen now even if the screensaver does not lock by default.
     */
    void lock();

    /**
     * Save the screen now. If the user has locking enabled, the screen is locked also.
     */
    void save();

    /**
     * Quit the screensaver if it is running
     */
    void quit();

    /**
     * Simulate user activity
     */
    void poke();

    /**
     * Return true if the screensaver is enabled
     */
    bool isEnabled();

    /**
     * Enable/disable the screensaver
     * @return true if the action succeeded
     */
    bool enable( bool e );

    /**
     * Return true if the screen is currently blanked
     */
    bool isBlanked();

    /**
     * Read and apply configuration.
     */
    void configure();

    /**
     * Enable or disable "blank only" mode.  This is useful for
     * laptops where one might not want a cpu thirsty screensaver
     * draining the battery.
     */
    void setBlankOnly( bool blankOnly );

    /**
     * Called by krunner_lock when locking is in effect.
     */
    void saverLockReady();

    /**
     * Request a change in the state of the screensaver.
     * Set to TRUE to request that the screensaver activate.
     * Active means that the screensaver has blanked the
     * screen and may run a graphical theme.  This does
     * not necessary mean that the screen is locked.
     */
    void setActive( bool state );

    /// Returns the value of the current state of activity (See setActive)
    bool getActive();

    /**
     * Returns the number of seconds that the screensaver has
     * been active.  Returns zero if the screensaver is not active.
     */
    quint32 getActiveTime();

    /// Returns the value of the current state of session idleness.
    bool getSessionIdle();

    /**
     * Returns the number of seconds that the session has
     * been idle.  Returns zero if the session is not idle.
     */
    quint32 getSessionIdleTime();

Q_SIGNALS:
    // DBus signals
    void screenSaverStarted();
    void screenSaverStopped();

protected Q_SLOTS:
    void idleTimeout();
    void lockProcessExited();

protected:
    enum LockType { DontLock, DefaultLock, ForceLock };
    bool startLockProcess( LockType lock_type );
    void stopLockProcess();
    bool handleKeyPress(XKeyEvent *xke);
    void processLockTransactions();
    xautolock_corner_t applyManualSettings(int);

private:
    enum State { Waiting, Preparing, Saving };
    bool        mEnabled;
    bool	mDPMS;

    State       mState;
    XAutoLock   *mXAutoLock;
    KProcess    mLockProcess;
    int		mTimeout;

    // the original X screensaver parameters
    int         mXTimeout;
    int         mXInterval;
    int         mXBlanking;
    int         mXExposures;

    time_t      m_actived_time;
    bool	mBlankOnly;  // only use the blanker, not the defined saver
    QDBusConnection screensaverService;
//    QVector< DCOPClientTransaction* > mLockTransactions;
};

#endif

