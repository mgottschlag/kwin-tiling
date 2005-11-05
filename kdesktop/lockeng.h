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
#include "KScreensaverIface.h"
#include "xautolock.h"
#include "xautolock_c.h"

//===========================================================================
/**
 * Screen saver engine.  Handles screensaver window, starting screensaver
 * hacks, and password entry.
 */
class SaverEngine
    : public QWidget,
      virtual public KScreensaverIface
{
    Q_OBJECT
public:
    SaverEngine();
    ~SaverEngine();

    /**
     * Lock the screen
     */
    virtual void lock();

    /**
     * Save the screen
     */
    virtual void save();

    /**
     * Quit the screensaver if running
     */
    virtual void quit();

    /**
     * return true if the screensaver is enabled
     */
    virtual bool isEnabled();

    /**
     * enable/disable the screensaver
     */
    virtual bool enable( bool e );

    /**
     * return true if the screen is currently blanked
     */
    virtual bool isBlanked();

    /**
     * Read and apply configuration.
     */
    virtual void configure();

    /**
     * Enable or disable "blank only" mode.  This is useful for
     * laptops where one might not want a cpu thirsty screensaver
     * draining the battery.
     */
    virtual void setBlankOnly( bool blankOnly );

protected slots:
    void idleTimeout();
    void lockProcessExited();

protected:
    enum LockType { DontLock, DefaultLock, ForceLock };
    void startLockProcess( LockType lock_type );
    void stopLockProcess();
    bool handleKeyPress(XKeyEvent *xke);
    xautolock_corner_t applyManualSettings(int);

protected:
    enum State { Waiting, Saving };
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
};

#endif

