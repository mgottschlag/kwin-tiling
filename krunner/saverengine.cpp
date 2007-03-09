//===========================================================================
//
// This file is part of the KDE project
//
// Copyright (c) 1999 Martin R. Jones <mjones@kde.org>
//


#include <config.h>

#include "saverengine.h"
#include "kscreensaversettings.h"
#include "screensaveradaptor.h"

#include <kstandarddirs.h>
#include <kapplication.h>
#include <kservicegroup.h>
#include <krandom.h>
#include <kdebug.h>
#include <klocale.h>
#include <QFile>
#include <QX11Info>
#include <assert.h>
#include <stdlib.h>
#include <time.h>

#include "xautolock_c.h"
extern xautolock_corner_t xautolock_corners[ 4 ];

//===========================================================================
//
// Screen saver engine. Doesn't handle the actual screensaver window,
// starting screensaver hacks, or password entry. That's done by
// a newly started process.
//
SaverEngine::SaverEngine()
    : QWidget(),
      screensaverService( QDBusConnection::connectToBus( QDBusConnection::SessionBus,
                                                         "org.freedesktop.ScreenSaver" ) )
{
    (void) new ScreenSaverAdaptor( this );
    screensaverService.registerService( "org.freedesktop.ScreenSaver" ) ;
    screensaverService.registerObject( "/ScreenSaver", this );

    // Save X screensaver parameters
    XGetScreenSaver(QX11Info::display(), &mXTimeout, &mXInterval,
                    &mXBlanking, &mXExposures);

    mState = Waiting;
    mXAutoLock = 0;
    mEnabled = false;

    m_nr_throttled = 0;
    m_nr_inhibited = 0;
    m_actived_time = -1;

    connect(&mLockProcess, SIGNAL(processExited(KProcess *)),
                        SLOT(lockProcessExited()));

    QObject::connect(QDBusConnection::sessionBus().interface(), SIGNAL(serviceOwnerChanged(QString,QString,QString)),
                SLOT(serviceOwnerChanged(QString,QString,QString)));

    // I make it a really random number to avoid
    // some assumptions in clients, but just increase
    // while gnome-ss creates a random number every time
    m_next_cookie = KRandom::random() % 20000;
    configure();
}

//---------------------------------------------------------------------------
//
// Destructor - usual cleanups.
//
SaverEngine::~SaverEngine()
{
    mLockProcess.detach(); // don't kill it if we crash
    delete mXAutoLock;

    // Restore X screensaver parameters
    XSetScreenSaver(QX11Info::display(), mXTimeout, mXInterval, mXBlanking,
                    mXExposures);
}

//---------------------------------------------------------------------------

// This should be called only using DBus.
void SaverEngine::Lock()
{
    bool ok = true;
    if (mState == Waiting)
    {
        ok = startLockProcess( ForceLock );
// It takes a while for krunner_lock to start and lock the screen.
// Therefore delay the DBus call until it tells krunner that the locking is in effect.
// This is done only for --forcelock .
        if( ok && mState != Saving )
        {
#ifdef __GNUC__
#warning port dcop transactions to dbus
#endif
#if 0
            DCOPClientTransaction* trans = kapp->dcopClient()->beginTransaction();
            mLockTransactions.append( trans );
#endif
        }
    }
    else
    {
        mLockProcess.kill( SIGHUP );
    }
}

void SaverEngine::processLockTransactions()
{
#ifdef __GNUC__
#warning port dcop transactions to dbus
#endif
#if 0
    for( QVector< DCOPClientTransaction* >::ConstIterator it = mLockTransactions.begin();
         it != mLockTransactions.end();
         ++it )
    {
        DCOPCString replyType = "void";
        QByteArray arr;
        kapp->dcopClient()->endTransaction( *it, replyType, arr );
    }
    mLockTransactions.clear();
#endif
}

void SaverEngine::saverLockReady()
{
    if( mState != Preparing )
    {
	kDebug() << "Got unexpected saverReady()" << endl;
    }
    kDebug() << "Saver Lock Ready" << endl;
    processLockTransactions();
}

void SaverEngine::SimulateUserActivity()
{
    if ( mState == Waiting )
    {
        // disable
        XSetScreenSaver(QX11Info::display(), 0, mXInterval, PreferBlanking, mXExposures);
        mXAutoLock->resetTrigger();
        // reenable
        XSetScreenSaver(QX11Info::display(), mTimeout + 10, mXInterval, PreferBlanking, mXExposures);
    }
}

//---------------------------------------------------------------------------
bool SaverEngine::save()
{
    if (mState == Waiting)
    {
        return startLockProcess( DefaultLock );
    }
    return false;
}

//---------------------------------------------------------------------------
bool SaverEngine::quit()
{
    if (mState == Saving || mState == Preparing)
    {
        stopLockProcess();
        return true;
    }
    return false;
}

//---------------------------------------------------------------------------
bool SaverEngine::isEnabled()
{
  return mEnabled;
}

//---------------------------------------------------------------------------
bool SaverEngine::enable( bool e )
{
    if ( e == mEnabled )
	return true;

    // If we aren't in a suitable state, we will not reconfigure.
    if (mState != Waiting)
        return false;

    mEnabled = e;

    if (mEnabled)
    {
	if ( !mXAutoLock ) {
	    mXAutoLock = new XAutoLock();
	    connect(mXAutoLock, SIGNAL(timeout()), SLOT(idleTimeout()));
	}
        mXAutoLock->setTimeout(mTimeout);
        mXAutoLock->setDPMS(mDPMS);
	//mXAutoLock->changeCornerLockStatus( mLockCornerTopLeft, mLockCornerTopRight, mLockCornerBottomLeft, mLockCornerBottomRight);

        // We'll handle blanking
        XSetScreenSaver(QX11Info::display(), mTimeout + 10, mXInterval, PreferBlanking, mXExposures);
        kDebug() << "XSetScreenSaver " << mTimeout + 10 << endl;

        mXAutoLock->start();

        kDebug() << "Saver Engine started, timeout: " << mTimeout << endl;
    }
    else
    {
	if (mXAutoLock)
	{
	    delete mXAutoLock;
	    mXAutoLock = 0;
	}

        XForceScreenSaver(QX11Info::display(), ScreenSaverReset );
        XSetScreenSaver(QX11Info::display(), 0, mXInterval,  PreferBlanking, DontAllowExposures);
        kDebug() << "Saver Engine disabled" << endl;
    }

    return true;
}

//---------------------------------------------------------------------------
bool SaverEngine::isBlanked()
{
  return (mState != Waiting);
}

//---------------------------------------------------------------------------
//
// Read and apply configuration.
//
void SaverEngine::configure()
{
    // If we aren't in a suitable state, we will not reconfigure.
    if (mState != Waiting)
        return;

    // create a new config obj to ensure we read the latest options
    KScreenSaverSettings::self()->readConfig();

    bool e  = KScreenSaverSettings::screenSaverEnabled();
    kDebug() << "enabled " << e << endl;
    mTimeout = KScreenSaverSettings::timeout();
    mDPMS = KScreenSaverSettings::dpmsDependent();

    mEnabled = !e;   // force the enable()

    int action;
    action = KScreenSaverSettings::actionTopLeft();
    xautolock_corners[0] = applyManualSettings(action);
    action = KScreenSaverSettings::actionTopRight();
    xautolock_corners[1] = applyManualSettings(action);
    action = KScreenSaverSettings::actionBottomLeft();
    xautolock_corners[2] = applyManualSettings(action);
    action = KScreenSaverSettings::actionBottomRight();
    xautolock_corners[3] = applyManualSettings(action);

    enable( e );
}

//---------------------------------------------------------------------------
//
// Start the screen saver.
//
bool SaverEngine::startLockProcess( LockType lock_type )
{
    if (mState != Waiting)
        return true;

    kDebug() << "SaverEngine: starting saver" << endl;
    emit ActiveChanged(true); // DBus signal

    if (mLockProcess.isRunning())
    {
        stopLockProcess();
    }
    mLockProcess.clearArguments();
    QString path = KStandardDirs::findExe( "krunner_lock" );
    if( path.isEmpty())
    {
	kDebug() << "Can't find krunner_lock!" << endl;
	return false;
    }
    mLockProcess << path;
    switch( lock_type )
    {
	case ForceLock:
    	    mLockProcess << QString( "--forcelock" );
	  break;
	case DontLock:
	    mLockProcess << QString( "--dontlock" );
	  break;
	default:
	  break;
    }
    if (m_nr_throttled)
        mLockProcess << QString( "--blank" );

    m_actived_time = time( 0 );
    if (mLockProcess.start() == false )
    {
	kDebug() << "Failed to start krunner_lock!" << endl;
        m_actived_time = -1;
	return false;
    }

    XSetScreenSaver(QX11Info::display(), 0, mXInterval,  PreferBlanking, mXExposures);
    mState = Preparing;
    if (mXAutoLock)
    {
        mXAutoLock->stop();
    }
    return true;
}

//---------------------------------------------------------------------------
//
// Stop the screen saver.
//
void SaverEngine::stopLockProcess()
{
    m_actived_time = -1;
    if (mState == Waiting)
    {
        kWarning() << "SaverEngine::stopSaver() saver not active" << endl;
        return;
    }
    kDebug() << "SaverEngine: stopping lock" << endl;
    emit ActiveChanged(false); // DBus signal

    mLockProcess.kill();

    if (mXAutoLock)
    {
        mXAutoLock->start();
    }
    processLockTransactions();
    mState = Waiting;
    XForceScreenSaver(QX11Info::display(), ScreenSaverReset );
    XSetScreenSaver(QX11Info::display(), mTimeout + 10, mXInterval, PreferBlanking, mXExposures);
}

void SaverEngine::lockProcessExited()
{
    m_actived_time = -1;
    kDebug() << "SaverEngine: lock exited" << endl;
    if( mState == Waiting )
	return;
    emit ActiveChanged(false); // DBus signal
    if (mXAutoLock)
    {
        mXAutoLock->start();
    }
    processLockTransactions();
    mState = Waiting;
    XForceScreenSaver(QX11Info::display(), ScreenSaverReset );
    XSetScreenSaver(QX11Info::display(), mTimeout + 10, mXInterval, PreferBlanking, mXExposures);
}

//---------------------------------------------------------------------------
//
// XAutoLock has detected the required idle time.
//
void SaverEngine::idleTimeout()
{
    // disable X screensaver
    XForceScreenSaver(QX11Info::display(), ScreenSaverReset );
    XSetScreenSaver(QX11Info::display(), 0, mXInterval, PreferBlanking, DontAllowExposures);
    startLockProcess( DefaultLock );
}

xautolock_corner_t SaverEngine::applyManualSettings(int action)
{
    if (action == 0)
    {
        kDebug() << "no lock" << endl;
        return ca_nothing;
    }
    else if (action == 1)
    {
        kDebug() << "lock screen" << endl;
        return ca_forceLock;
    }
    else if (action == 2)
    {
        kDebug() << "prevent lock" << endl;
        return ca_dontLock;
    }
    else
    {
        kDebug() << "no lock nothing" << endl;
        return ca_nothing;
    }
}

uint SaverEngine::GetSessionIdleTime()
{
    return mXAutoLock->idleTime();
}

bool SaverEngine::GetSessionIdle()
{
    // pointless?
    return ( GetSessionIdleTime() > 0 );
}

uint SaverEngine::GetActiveTime()
{
    if ( m_actived_time == -1 )
        return 0;
    return time( 0 ) - m_actived_time;
}

bool SaverEngine::GetActive()
{
    return ( mState != Waiting );
}

bool SaverEngine::SetActive(bool state)
{
    if ( state )
        return save();
    else
        return quit();
}

uint SaverEngine::Inhibit(const QString &application_name, const QString &reason_for_inhibit)
{
    ScreenSaverRequest sr;
    sr.appname = application_name;
    sr.reasongiven = reason_for_inhibit;
    sr.cookie = m_next_cookie++;
    sr.dbusid = "unknown"; // see below for throttle
    sr.type = ScreenSaverRequest::Inhibit;
    m_requests.append( sr );
    m_nr_inhibited++;
    enable( false );
    return sr.cookie;
}

void SaverEngine::UnInhibit(uint cookie)
{
    QMutableListIterator<ScreenSaverRequest> it( m_requests );
    while ( it.hasNext() )
    {
        if ( it.next().cookie == cookie ) {
            it.remove();
            m_nr_inhibited--;
            if ( m_nr_inhibited <= 0 )
                enable( true );
        }
    }
}

uint SaverEngine::Throttle(const QString &application_name, const QString &reason_for_inhibit)
{
    ScreenSaverRequest sr;
    sr.appname = application_name;
    sr.reasongiven = reason_for_inhibit;
    sr.cookie = m_next_cookie++;
    sr.type = ScreenSaverRequest::Throttle;
#warning thiago says Qt 4.3 can query the dbus connection id in adaptors - waiting
    sr.dbusid = "unknown"; // see above for inhibit
    m_requests.append( sr );
    m_nr_throttled++;
    mLockProcess.suspend();
    return sr.cookie;
}

void SaverEngine::UnThrottle(uint cookie)
{
    QMutableListIterator<ScreenSaverRequest> it( m_requests );
    while ( it.hasNext() )
    {
        if ( it.next().cookie == cookie ) {
            it.remove();
            m_nr_throttled--;
            if ( m_nr_throttled <= 0 )
                mLockProcess.resume();
        }
    }
}

void SaverEngine::serviceOwnerChanged(const QString& name,const QString &oldOwner,const QString &newOwner)
{
    Q_UNUSED( oldOwner );

    if ( !newOwner.isEmpty() ) // looking for deaths
        return;

    QListIterator<ScreenSaverRequest> it( m_requests );
    while ( it.hasNext() )
    {
        ScreenSaverRequest r = it.next();
        if ( r.dbusid == name )
        {
            if ( r.type == ScreenSaverRequest::Throttle )
                UnThrottle( r.cookie );
            else
                UnInhibit( r.cookie );
        }
    }
}

#include "saverengine.moc"


