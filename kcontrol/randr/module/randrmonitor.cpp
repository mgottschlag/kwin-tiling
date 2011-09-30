/********************************************************************

Copyright (C) 2008 Lubos Lunak <l.lunak@suse.cz>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*********************************************************************/

#include "randrmonitor.h"

#include <kaction.h>
#include <kactioncollection.h>
#include <kapplication.h>
#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kpluginfactory.h>
#include <kpluginloader.h>
#include <ktoolinvocation.h>

#include <qdbusconnection.h>
#include <qdbusconnectioninterface.h>
#include <qtimer.h>
#include <qx11info_x11.h>

K_PLUGIN_FACTORY(RandrMonitorModuleFactory,
                 registerPlugin<RandrMonitorModule>();
    )
K_EXPORT_PLUGIN(RandrMonitorModuleFactory("randrmonitor"))

RandrMonitorModule::RandrMonitorModule( QObject* parent, const QList<QVariant>& )
    : KDEDModule( parent )
    , have_randr( false )
    {
    setModuleName( "randrmonitor" );
    initRandr();
    QDBusConnection::sessionBus().connect("org.kde.Solid.PowerManagement", "/org/kde/Solid/PowerManagement", "org.kde.Solid.PowerManagement", "resumingFromSuspend", this, SLOT(resumedFromSuspend()));
    }

RandrMonitorModule::~RandrMonitorModule()
    {
    if( have_randr )
        {
        Display* dpy = QX11Info::display();
        XDestroyWindow( dpy, window );
        delete helper;
        delete dialog;
        have_randr = false;
        }
    }

void RandrMonitorModule::initRandr()
    {
    Display* dpy = QX11Info::display();
    if( !XRRQueryExtension( dpy, &randr_base, &randr_error ))
        return;
    int major = 1;
    int minor = 2;
    if( !XRRQueryVersion( dpy, &major, &minor ) || major < 1 || (major == 1 && minor < 2 ))
        return;
    have_randr = true;
    // It looks like we need a separate window for getting the events, so that we don't
    // change e.g. Qt's event mask.
    window = XCreateSimpleWindow( dpy, DefaultRootWindow( dpy ), 0, 0, 1, 1, 0, 0, 0 );
    XRRSelectInput( dpy, window, RROutputChangeNotifyMask );
#if 0 // xrandr apparently can't detect hw changes and on some systems polling freezes X :(
    // HACK: see poll()
    QTimer* timer = new QTimer( this );
    timer->start( 10000 ); // 10 s
    connect( timer, SIGNAL(timeout()), this, SLOT(poll()));
#endif
    helper = new RandrMonitorHelper( this );
    kapp->installX11EventFilter( helper );
    dialog = NULL;
    currentMonitors = connectedMonitors();
    KActionCollection* coll = new KActionCollection( this );
    KAction* act = coll->addAction( "display" );
    act->setText( i18n( "Switch Display" ));
    act->setGlobalShortcut( KShortcut( Qt::Key_Display ));
    connect( act, SIGNAL(triggered(bool)), SLOT(switchDisplay()));
    }

void RandrMonitorModule::poll()
    {
    // HACK: It seems that RRNotify/RRNotify_OutputChange event (i.e. detecting a newly
    // plugged or unplugged monitor) does not work without polling some randr functionality.
    int dummy;
    XRRGetScreenSizeRange( QX11Info::display(), window, &dummy, &dummy, &dummy, &dummy );
    }

void RandrMonitorModule::processX11Event( XEvent* e )
    {
    if( e->xany.type == randr_base + RRNotify )
        {
        XRRNotifyEvent* e2 = reinterpret_cast< XRRNotifyEvent* >( e );
        if( e2->subtype == RRNotify_OutputChange ) // TODO && e2->window == window )
            {
            kDebug() << "Monitor change detected";
            QStringList newMonitors = connectedMonitors();
            if( newMonitors == currentMonitors )
                return;
            if( QDBusConnection::sessionBus().interface()->isServiceRegistered(
                "org.kde.internal.KSettingsWidget-kcm_randr" ))
                { // already running
                return;
                }
            kapp->updateUserTimestamp(); // well, let's say plugging in a monitor is a user activity
#warning Modal dialog, stupid, fix.
            QString change;
            QString question =
                ( newMonitors.count() < currentMonitors.count()
                    ? i18n( "A monitor output has been disconnected." )
                    : i18n( "A new monitor output has been connected." ))
                + "\n\n" + i18n( "Do you wish to run a configuration tool to adjust the monitor setup?" );
            currentMonitors = newMonitors;
            if( KMessageBox::questionYesNo( NULL, question, i18n( "Monitor setup has changed" ),
                    KGuiItem( i18n( "Con&figure" ) ), KGuiItem( i18n( "&Ignore" ) ), "randrmonitorchange" )
                == KMessageBox::Yes )
                {
                KToolInvocation::kdeinitExec( "kcmshell4", QStringList() << "display" );
                }
            }
        }
    }

QStringList RandrMonitorModule::connectedMonitors() const
    {
    QStringList ret;
    Display* dpy = QX11Info::display();
    XRRScreenResources* resources = XRRGetScreenResources( dpy, window );
    for( int i = 0;
         i < resources->noutput;
         ++i )
        {
        XRROutputInfo* info = XRRGetOutputInfo( dpy, resources, resources->outputs[ i ] );
        QString name = QString::fromUtf8( info->name );
        if( info->connection == RR_Connected )
            ret.append( name );
        XRRFreeOutputInfo( info );
        }
    XRRFreeScreenResources( resources );
    return ret;
    }

void RandrMonitorModule::switchDisplay()
    {
    QList< RandROutput* > outputs;
    RandRDisplay display;
    outputs = connectedOutputs( display );
    if( outputs.count() == 0 ) // nothing connected, do nothing
        return;
    if( outputs.count() == 1 ) // just one, enable it
        {
        enableOutput( outputs[0], true );
        for( int scr = 0; scr < display.numScreens(); ++scr )
            {
            foreach( RandROutput* output, display.screen( scr )->outputs())
                {
                if( !output->isConnected())
                    enableOutput( output, false ); // switch off every output that's not connected
                }
            }
        return;
        }
    if( outputs.count() == 2 ) // alternative between one, second, both
        {
        if( outputs[ 0 ]->isActive() && !outputs[ 1 ]->isActive())
            {
            enableOutput( outputs[ 1 ], true );
            enableOutput( outputs[ 0 ], false );
            }
        else if( !outputs[ 0 ]->isActive() && outputs[ 1 ]->isActive())
            {
            enableOutput( outputs[ 1 ], true );
            enableOutput( outputs[ 0 ], true );
            }
        else
            {
            enableOutput( outputs[ 0 ], true );
            enableOutput( outputs[ 1 ], false );
            }
        return;
        }
    // no idea what to do here
    KToolInvocation::kdeinitExec( "kcmshell4", QStringList() << "display" );
    }

void RandrMonitorModule::resumedFromSuspend()
    {
    RandRDisplay display;
    QList< RandROutput* > m_connectedOutputs, m_validCrtcOutputs;
    m_connectedOutputs = connectedOutputs( display );
    m_validCrtcOutputs = validCrtcOutputs( display );
    if( m_connectedOutputs.count() == 0 )
        return;
    // We have at least one connected output.
    // We check all outputs with valid crtc if they are still connected.
    // If not, we are going to disable them.
    QList<RandROutput*> outputsToDisable;
    foreach( RandROutput* output, m_validCrtcOutputs )
        {
        if( !output->isConnected() )
            outputsToDisable.append( output );
        }
    // If no active output is still connected we are going to enable the first connected output.
    if( outputsToDisable.size() == m_validCrtcOutputs.size() )
        enableOutput( m_connectedOutputs[0], true);
    // Now we can disable the disconnected outputs
    foreach( RandROutput* output, outputsToDisable)
        {
        enableOutput( output, false );
        }
    }

void RandrMonitorModule::enableOutput( RandROutput* output, bool enable )
    { // a bit lame, but I don't know how to do this easily with this codebase :-/
    KProcess::execute( QStringList() << "xrandr" << "--output" << output->name() << ( enable ? "--auto" : "--off" ));
    }

QList< RandROutput* > RandrMonitorModule::connectedOutputs( RandRDisplay &display )
    {
    return outputs( display, true, false, false );
    }

QList< RandROutput* > RandrMonitorModule::activeOutputs( RandRDisplay &display )
    {
    return outputs( display, false, true, false );
    }

QList< RandROutput* > RandrMonitorModule::validCrtcOutputs( RandRDisplay &display )
    {
    return outputs( display, false, false, true );
    }

QList< RandROutput* > RandrMonitorModule::outputs( RandRDisplay &display, bool connected, bool active, bool validCrtc )
    {
    QList< RandROutput* > outputs;
    for( int scr = 0; scr < display.numScreens(); ++scr )
        {
        foreach( RandROutput* output, display.screen( scr )->outputs() )
            {
            if( !output->isConnected() && connected )
                continue;
            if( !output->isActive() && active )
                continue;
            if( !output->crtc()->isValid() && validCrtc )
                continue;
            if( !outputs.contains( output ) )
                outputs.append( output );
            }
        }
    return outputs;
    }

bool RandrMonitorHelper::x11Event( XEvent* e )
    {
    module->processX11Event( e );
    return QWidget::x11Event( e );
    }

#include "randrmonitor.moc"
