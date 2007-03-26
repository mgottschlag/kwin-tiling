//===========================================================================
//
// This file is part of the KDE project
//
// Copyright (c) 1999 Martin R. Jones <mjones@kde.org>
//

#ifndef __SAVERENGINE_H__
#define __SAVERENGINE_H__

#include <QWidget>
#include <k3process.h>
#include <QVector>
#include <QDBusConnection>

#include "xautolock.h"
#include "xautolock_c.h"

class ScreenSaverRequest
{
public:
    QString appname;
    QString reasongiven;
    QString dbusid;
    uint cookie;
    enum { Inhibit,Throttle } type;
};

//===========================================================================
/**
 * Screen saver engine.  Handles screensaver window, starting screensaver
 * hacks, and password entry.
 */
class SaverEngine : public QWidget
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.freedesktop.ScreenSaver")

public:
    SaverEngine();
    ~SaverEngine();

public Q_SLOTS:
    /**
     * Lock the screen now even if the screensaver does not lock by default.
     */
    void Lock();

    /**
     * Save the screen now. If the user has locking enabled, the screen is locked also.
     */
    bool save();

    /**
     * Quit the screensaver if it is running
     */
    bool quit();

    /**
     * Simulate user activity
     */
    void SimulateUserActivity();

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
    bool SetActive( bool state );

    /// Returns the value of the current state of activity (See setActive)
    bool GetActive();

    /**
     * Returns the number of seconds that the screensaver has
     * been active.  Returns zero if the screensaver is not active.
     */
    uint GetActiveTime();

    /// Returns the value of the current state of session idleness.
    bool GetSessionIdle();

    /**
     * Returns the number of seconds that the session has
     * been idle.  Returns zero if the session is not idle.
     */
    uint GetSessionIdleTime();

    /**
     * Request that saving the screen due to system idleness
     * be blocked until UnInhibit is called or the
     * calling process exits.
     * The cookie is a random number used to identify the request
     */
    uint Inhibit(const QString &application_name, const QString &reason_for_inhibit);
    /// Cancel a previous call to Inhibit() identified by the cookie.
    void UnInhibit(uint cookie);

    /**
     * Request that running themes while the screensaver is active
     * be blocked until UnThrottle is called or the
     * calling process exits.
     * The cookie is a random number used to identify the request
     */
    uint Throttle(const QString &application_name, const QString &reason_for_inhibit);
    /// Cancel a previous call to Throttle() identified by the cookie.
    void UnThrottle(uint cookie);

Q_SIGNALS:
    // DBus signals
    void ActiveChanged(bool state);

protected Q_SLOTS:
    void idleTimeout();
    void lockProcessExited();
    void serviceOwnerChanged(const QString&,const QString&,const QString&);

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
    K3Process    mLockProcess;
    int		mTimeout;

    // the original X screensaver parameters
    int         mXTimeout;
    int         mXInterval;
    int         mXBlanking;
    int         mXExposures;

    time_t      m_actived_time;
    QDBusConnection screensaverService;
    QList<ScreenSaverRequest> m_requests;
    uint        m_next_cookie;
    
    int        m_nr_throttled;
    int        m_nr_inhibited;
 
//    QVector< DCOPClientTransaction* > mLockTransactions;
};

#endif

