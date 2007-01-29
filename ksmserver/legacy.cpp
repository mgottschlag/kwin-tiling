/*****************************************************************
ksmserver - the KDE session management server

Copyright (C) 2000 Matthias Ettrich <ettrich@kde.org>
Copyright (C) 2005 Lubos Lunak <l.lunak@kde.org>

relatively small extensions by Oswald Buddenhagen <ob6@inf.tu-dresden.de>

some code taken from the dcopserver (part of the KDE libraries), which is
Copyright (c) 1999 Matthias Ettrich <ettrich@kde.org>
Copyright (c) 1999 Preston Brown <pbrown@kde.org>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

******************************************************************/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

#include "server.h"

#include <unistd.h>

#include <QTimer>

#include <kconfig.h>
#include <kdebug.h>
#include <kwinmodule.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <kconfiggroup.h>

/*
* Legacy session management
*/

#ifndef NO_LEGACY_SESSION_MANAGEMENT
const int WM_SAVE_YOURSELF_TIMEOUT = 4000;

static WindowMap* windowMapPtr = 0;

static Atom wm_save_yourself = XNone;
static Atom wm_protocols = XNone;
static Atom wm_client_leader = XNone;
static Atom sm_client_id = XNone;

static int winsErrorHandler(Display *, XErrorEvent *ev)
{
    if (windowMapPtr) {
        WindowMap::Iterator it = windowMapPtr->find(ev->resourceid);
        if (it != windowMapPtr->end())
            (*it).type = SM_ERROR;
    }
    return 0;
}

void KSMServer::performLegacySessionSave()
{
    kDebug( 1218 ) << "Saving legacy session apps" << endl;
    // Setup error handler
    legacyWindows.clear();
    windowMapPtr = &legacyWindows;
    XErrorHandler oldHandler = XSetErrorHandler(winsErrorHandler);
    // Compute set of leader windows that need legacy session management
    // and determine which style (WM_COMMAND or WM_SAVE_YOURSELF)
    KWinModule module;
    if( wm_save_yourself == (Atom)XNone ) {
        Atom atoms[ 4 ];
        const char* const names[]
            = { "WM_SAVE_YOURSELF", "WM_PROTOCOLS", "WM_CLIENT_LEADER", "SM_CLIENT_ID" };
        XInternAtoms( QX11Info::display(), const_cast< char** >( names ), 4,
            False, atoms );
        wm_save_yourself = atoms[ 0 ];
        wm_protocols = atoms[ 1 ];
        wm_client_leader = atoms[ 2 ];
        sm_client_id = atoms[ 3 ];
    }
    for ( QList<WId>::ConstIterator it = module.windows().begin();
        it != module.windows().end(); ++it) {
        WId leader = windowWmClientLeader( *it );
        if (!legacyWindows.contains(leader) && windowSessionId( *it, leader ).isEmpty()) {
            SMType wtype = SM_WMCOMMAND;
            int nprotocols = 0;
            Atom *protocols = 0;
            if( XGetWMProtocols(QX11Info::display(), leader, &protocols, &nprotocols)) {
                for (int i=0; i<nprotocols; i++)
                    if (protocols[i] == wm_save_yourself) {
                        wtype = SM_WMSAVEYOURSELF;
                        break;
                    }
                XFree((void*) protocols);
            }
            SMData data;
            data.type = wtype;
            XClassHint classHint;
            if( XGetClassHint( QX11Info::display(), leader, &classHint ) ) {
                data.wmclass1 = classHint.res_name;
                data.wmclass2 = classHint.res_class;
                XFree( classHint.res_name );
                XFree( classHint.res_class );
            }
            legacyWindows.insert(leader, data);
        }
    }
    // Open fresh display for sending WM_SAVE_YOURSELF
    XSync(QX11Info::display(), False);
    Display *newdisplay = XOpenDisplay(DisplayString(QX11Info::display()));
    if (!newdisplay) {
        windowMapPtr = NULL;
        XSetErrorHandler(oldHandler);
        return;
    }
    WId root = DefaultRootWindow(newdisplay);
    XGrabKeyboard(newdisplay, root, False,
                GrabModeAsync, GrabModeAsync, CurrentTime);
    XGrabPointer(newdisplay, root, False, Button1Mask|Button2Mask|Button3Mask,
                GrabModeAsync, GrabModeAsync, XNone, XNone, CurrentTime);
    // Send WM_SAVE_YOURSELF messages
    XEvent ev;
    int awaiting_replies = 0;
    for (WindowMap::Iterator it = legacyWindows.begin(); it != legacyWindows.end(); ++it) {
        if ( (*it).type == SM_WMSAVEYOURSELF ) {
            WId w = it.key();
            awaiting_replies += 1;
            memset(&ev, 0, sizeof(ev));
            ev.xclient.type = ClientMessage;
            ev.xclient.window = w;
            ev.xclient.message_type = wm_protocols;
            ev.xclient.format = 32;
            ev.xclient.data.l[0] = wm_save_yourself;
            ev.xclient.data.l[1] = QX11Info::appTime();
            XSelectInput(newdisplay, w, PropertyChangeMask|StructureNotifyMask);
            XSendEvent(newdisplay, w, False, 0, &ev);
        }
    }
    // Wait for change in WM_COMMAND with timeout
    XFlush(newdisplay);
    QTime start = QTime::currentTime();
    while (awaiting_replies > 0) {
        if (XPending(newdisplay)) {
            /* Process pending event */
            XNextEvent(newdisplay, &ev);
            if ( ( ev.xany.type == UnmapNotify ) ||
                ( ev.xany.type == PropertyNotify && ev.xproperty.atom == XA_WM_COMMAND ) ) {
                WindowMap::Iterator it = legacyWindows.find( ev.xany.window );
                if ( it != legacyWindows.end() && (*it).type != SM_WMCOMMAND ) {
                    awaiting_replies -= 1;
                    if ( (*it).type != SM_ERROR )
                        (*it).type = SM_WMCOMMAND;
                }
            }
        } else {
            /* Check timeout */
            int msecs = start.elapsed();
            if (msecs >= WM_SAVE_YOURSELF_TIMEOUT)
                break;
            /* Wait for more events */
            fd_set fds;
            FD_ZERO(&fds);
            int fd = ConnectionNumber(newdisplay);
            FD_SET(fd, &fds);
            struct timeval tmwait;
            tmwait.tv_sec = (WM_SAVE_YOURSELF_TIMEOUT - msecs) / 1000;
            tmwait.tv_usec = ((WM_SAVE_YOURSELF_TIMEOUT - msecs) % 1000) * 1000;
            ::select(fd+1, &fds, NULL, &fds, &tmwait);
        }
    }
    // Terminate work in new display
    XAllowEvents(newdisplay, ReplayPointer, CurrentTime);
    XAllowEvents(newdisplay, ReplayKeyboard, CurrentTime);
    XSync(newdisplay, False);
    XCloseDisplay(newdisplay);
    // Restore old error handler
    XSync(QX11Info::display(), False);
    XSetErrorHandler(oldHandler);
    for (WindowMap::Iterator it = legacyWindows.begin(); it != legacyWindows.end(); ++it) {
        if ( (*it).type != SM_ERROR) {
            WId w = it.key();
            (*it).wmCommand = windowWmCommand(w);
            (*it).wmClientMachine = windowWmClientMachine(w);
        }
    }
    kDebug( 1218 ) << "Done saving " << legacyWindows.count() << " legacy session apps" << endl;
}

/*!
Stores legacy session management data
*/
void KSMServer::storeLegacySession( KConfig* config )
{
    // Write LegacySession data
    config->deleteGroup( "Legacy" + sessionGroup );
    KConfigGroup group( config, "Legacy" + sessionGroup );
    int count = 0;
    for (WindowMap::ConstIterator it = legacyWindows.begin(); it != legacyWindows.end(); ++it) {
        if ( (*it).type != SM_ERROR) {
            if( excludeApps.contains( (*it).wmclass1.toLower())
                || excludeApps.contains( (*it).wmclass2.toLower()))
                continue;
            if ( !(*it).wmCommand.isEmpty() && !(*it).wmClientMachine.isEmpty() ) {
                count++;
                QString n = QString::number(count);
                group.writeEntry( QString("command")+n, (*it).wmCommand );
                group.writeEntry( QString("clientMachine")+n, (*it).wmClientMachine );
            }
        }
    }
    group.writeEntry( "count", count );
}

/*!
Restores legacy session management data (i.e. restart applications)
*/
void KSMServer::restoreLegacySession( KConfig* config )
{
    if( config->hasGroup( "Legacy" + sessionGroup )) {
        KConfigGroup group( config, "Legacy" + sessionGroup );
        restoreLegacySessionInternal( config );
    } else if( wm == "kwin" ) { // backwards comp. - get it from kwinrc
        KConfigGroup group( config, sessionGroup );
        int count =  group.readEntry( "count", 0 );
        for ( int i = 1; i <= count; i++ ) {
            QString n = QString::number(i);
            if ( group.readEntry( QString("program")+n, QString() ) != wm )
                continue;
            QStringList restartCommand =
                config->readEntry( QString("restartCommand")+n, QStringList() );
            for( QStringList::ConstIterator it = restartCommand.begin();
                it != restartCommand.end();
                ++it ) {
                if( (*it) == "-session" ) {
                    ++it;
                    if( it != restartCommand.end()) {
                        KConfig cfg( "session/" + wm + '_' + (*it), true );
                        cfg.setGroup( "LegacySession" );
                        restoreLegacySessionInternal( &cfg, ' ' );
                    }
                }
            }
        }
    }
}

void KSMServer::restoreLegacySessionInternal( KConfig* config, char sep )
{
    int count = config->readEntry( "count",0 );
    for ( int i = 1; i <= count; i++ ) {
        QString n = QString::number(i);
        QStringList wmCommand = config->readEntry( QString("command")+n, QStringList(), sep );
        startApplication( wmCommand,
                        config->readEntry( QString("clientMachine")+n, QString() ),
                        config->readEntry( QString("userId")+n, QString() ));
    }
}

static QByteArray getQCStringProperty(WId w, Atom prop)
{
    Atom type;
    int format, status;
    unsigned long nitems = 0;
    unsigned long extra = 0;
    unsigned char *data = 0;
    QByteArray result = "";
    status = XGetWindowProperty( QX11Info::display(), w, prop, 0, 10000,
                                false, XA_STRING, &type, &format,
                                &nitems, &extra, &data );
    if ( status == Success) {
        if( data )
            result = (char*)data;
        XFree(data);
    }
    return result;
}

static QStringList getQStringListProperty(WId w, Atom prop)
{
    Atom type;
    int format, status;
    unsigned long nitems = 0;
    unsigned long extra = 0;
    unsigned char *data = 0;
    QStringList result;

    status = XGetWindowProperty( QX11Info::display(), w, prop, 0, 10000,
                                false, XA_STRING, &type, &format,
                                &nitems, &extra, &data );
    if ( status == Success) {
        if (!data)
            return result;
        for (int i=0; i<(int)nitems; i++) {
            result << QLatin1String( (const char*)data + i );
            while(data[i]) i++;
        }
        XFree(data);
    }
    return result;
}

QStringList KSMServer::windowWmCommand(WId w)
{
    QStringList ret = getQStringListProperty(w, XA_WM_COMMAND);
    // hacks here
    if( ret.count() == 1 ) {
        QString command = ret.first();
        // Mozilla is launched using wrapper scripts, so it's launched using "mozilla",
        // but the actual binary is "mozilla-bin" or "<path>/mozilla-bin", and that's what
        // will be also in WM_COMMAND - using this "mozilla-bin" doesn't work at all though
        if( command.endsWith( "mozilla-bin" ))
            return QStringList() << "mozilla";
        if( command.endsWith( "firefox-bin" ))
            return QStringList() << "firefox";
        if( command.endsWith( "thunderbird-bin" ))
            return QStringList() << "thunderbird";
    }
    return ret;
}

QString KSMServer::windowWmClientMachine(WId w)
{
    QByteArray result = getQCStringProperty(w, XA_WM_CLIENT_MACHINE);
    if (result.isEmpty()) {
        result = "localhost";
    } else {
        // special name for the local machine (localhost)
        char hostnamebuf[80];
        if (gethostname (hostnamebuf, sizeof hostnamebuf) >= 0) {
            hostnamebuf[sizeof(hostnamebuf)-1] = 0;
            if (result == hostnamebuf)
                result = "localhost";
            if(char *dot = strchr(hostnamebuf, '.')) {
                *dot = '\0';
                if(result == hostnamebuf)
                    result = "localhost";
            }
        }
    }
    return QLatin1String(result);
}

WId KSMServer::windowWmClientLeader(WId w)
{
    Atom type;
    int format, status;
    unsigned long nitems = 0;
    unsigned long extra = 0;
    unsigned char *data = 0;
    Window result = w;
    status = XGetWindowProperty( QX11Info::display(), w, wm_client_leader, 0, 10000,
                                false, XA_WINDOW, &type, &format,
                                &nitems, &extra, &data );
    if (status  == Success ) {
        if (data && nitems > 0)
            result = *((Window*) data);
        XFree(data);
    }
    return result;
}


/*
Returns sessionId for this client,
taken either from its window or from the leader window.
*/
QByteArray KSMServer::windowSessionId(WId w, WId leader)
{
    QByteArray result = getQCStringProperty(w, sm_client_id);
    if (result.isEmpty() && leader != (WId)None && leader != w)
        result = getQCStringProperty(leader, sm_client_id);
    return result;
}
#endif
