/********************************************************************

Copyright (C) 2008 Lubos Lunak <l.lunak@suse.cz>

Please see file LICENSE for the licensing terms of ksplashx as a whole.

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

#include <kapplication.h>
#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kpluginfactory.h>
#include <kpluginloader.h>
#include <ktoolinvocation.h>

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
    // HACK: see poll()
    QTimer* timer = new QTimer( this );
    timer->start( 10000 ); // 10 s
    connect( timer, SIGNAL( timeout()), this, SLOT( poll()));
    helper = new RandrMonitorHelper( this );
    kapp->installX11EventFilter( helper );
    dialog = NULL;
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
            kdDebug() << "WIN:" << e2->window << window << DefaultRootWindow( QX11Info::display());
            // TODO nedelat nic, kdyz kcmshell4 display uz je otevreny
            XRROutputChangeNotifyEvent* e3 = reinterpret_cast< XRROutputChangeNotifyEvent* >( e2 );
            // well, let's say plugging in a monitor is a user activity
            kapp->updateUserTimestamp();
#warning Modal dialog, stupid, fix.
            // TODO musi se zobrazit na spravnem monitoru (tj. ne na vypnutem)
            // TODO zkontrolovat, ze tohle opravdu nerusit randr eventmask pro Qt
            QString change;
            QRect availableRect;
            getRandrInfo( e3, &change, &availableRect );
            // TODO improve the text
            QString question;
            question = change + "\n"
                + i18n( "Do you wish to run a configuration tool to adjust the monitor setup?" );
            if( KMessageBox::questionYesNo( NULL, question, i18n( "Monitor setup has changed" ),
                    KGuiItem( "Con&figure" ), KGuiItem( "&Ignore" ), "randrmonitorchange" )
                == KMessageBox::Yes )
                {
                KToolInvocation::kdeinitExec( "kcmshell4", QStringList() << "display" );
                }
            }
        }
    }

void RandrMonitorModule::getRandrInfo( XRROutputChangeNotifyEvent* e, QString* change, QRect* rect )
    {
    Display* dpy = QX11Info::display();
    XRRScreenResources* resources = XRRGetScreenResources( dpy, window );
    XRROutputInfo* info = XRRGetOutputInfo( dpy, resources, e->output );
    if( e->connection == RR_Connected )
        *change = i18n( "A new monitor output '%1' has been connected.", QString::fromUtf8( info->name ));
    else
        *change = i18n( "Monitor output '%1' has been disconnected.", QString::fromUtf8( info->name ));
    XRRFreeOutputInfo( info );
    XRRFreeScreenResources( resources );
    }

bool RandrMonitorHelper::x11Event( XEvent* e )
    {
    module->processX11Event( e );
    return QWidget::x11Event( e );
    }

#include "randrmonitor.moc"
