//===========================================================================
//
// This file is part of the KDE project
//
// Copyright (c) 1999 Martin R. Jones <mjones@kde.org>
//

#ifndef __LOCKENG_H__
#define __LOCKENG_H__

#include <QWidget>
#include <kprocess.h>
#include <QVector>

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
public:
    SaverEngine();
    ~SaverEngine();

    // DBus exported methods

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
     * Called by kdesktop_lock when locking is in effect.
     */
    void saverLockReady();

    static const char* screenSaverDBusObjectPath;

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

protected:
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

    bool	mBlankOnly;  // only use the blanker, not the defined saver
//    QVector< DCOPClientTransaction* > mLockTransactions;
};

#endif

