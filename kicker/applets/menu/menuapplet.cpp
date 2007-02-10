/*****************************************************************

Copyright (c) 2002 Siegfried Nijssen <snijssen@liacs.nl>
Copyright (c) 2003 Lubos Lunak <l.lunak@suse.cz>

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

#define _MENUAPPLET_CPP_

#include "menuapplet.h"

#include <QLayout>
#include <QToolTip>
#include <QVariant> // avoid X11 #define's
#include <QList>

#include <kapplication.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <klocale.h>
#include <kwin.h>
#include <kwinmodule.h>

#include <netwm.h>

#include <X11/Xlib.h>
#include <QX11Info>

#include <stdio.h>

/*

 KMenuBar from KDE3.1 and older won't work very well with this applet.
 This is because QMenuBar tries really hard to keep its preffered size,
 se even if the X window for the menubar has the size enforced by this
 applet, Qt thinks it has the size Qt wants. This results in parts
 of the menubar not being repainted. Also, old KMenuBar always forced
 with to be the width of the screen, so even if the menubar has only
 few entries, this applet will still indicate the menubar doesn't
 fit completely in it. There's no way to fix this, besides upgrading
 to KDE3.2.

*/



extern "C"
{
    KDE_EXPORT KPanelApplet* init( QWidget* parent_P, const QString& configFile_P )
    {
      KGlobal::locale()->insertCatalog("kmenuapplet");
      return new KickerMenuApplet::Applet( configFile_P, parent_P );
    }
}

namespace KickerMenuApplet
{

static const int MOVE_DIFF = 100; // size increment for left/right menu moving
static const int GROW_WIDTH = 10; // width of grow buttons

const long SUPPORTED_WINDOW_TYPES = NET::NormalMask | NET::DesktopMask | NET::DockMask
                | NET::ToolbarMask | NET::MenuMask | NET::DialogMask | NET::OverrideMask
                | NET::TopMenuMask | NET::UtilityMask | NET::SplashMask;

Applet::Applet( const QString& configFile_P, QWidget* parent_P )
    :   KPanelApplet( configFile_P, Plasma::Normal, 0, parent_P ),
        //DCOPObject( "menuapplet" ),
        module( NULL ),
        active_menu( NULL ),
        selection( NULL ),
        selection_watcher( NULL ),
        desktop_menu( false ),
        topEdgeOffset( 0 )
    {
        //dcopclient.registerAs( "menuapplet", false );
    // toolbarAppearanceChanged(int) is sent when changing macstyle
    connect( KGlobalSettings::self(), SIGNAL( toolbarAppearanceChanged( int )),
        this, SLOT( readSettings()));
    claimSelection();
    readSettings();
    updateTopEdgeOffset();
    }

Applet::~Applet()
    {
    lostSelection(); // release all menu's before really loosing the selection
    delete selection;
    delete selection_watcher;
    delete module;
    KGlobal::locale()->removeCatalog("kmenuapplet");
    }

void Applet::windowAdded( WId w_P )
    {
    NETWinInfo info( QX11Info::display(), w_P, QX11Info::appRootWindow(), NET::WMWindowType );
    if( info.windowType( SUPPORTED_WINDOW_TYPES ) != NET::TopMenu )
	return;
//    kDebug() << "embedding:" << w_P << endl;
    Window transient_for = KWin::transientFor( w_P );
    if( transient_for == None )
	return;
    MenuEmbed* embed;
    if( transient_for == QX11Info::appRootWindow())
    {
        embed = new MenuEmbed( transient_for, true, this );
    }
    else
        {
        KWin::WindowInfo info2 = KWin::windowInfo( transient_for, NET::WMWindowType );
        embed = new MenuEmbed( transient_for,
            info2.windowType( SUPPORTED_WINDOW_TYPES ) == NET::Desktop, this );
        }
        embed->hide();
    embed->move( 0, -topEdgeOffset );
    embed->resize( embed->width(), height() + topEdgeOffset );
    embed->embedInto( w_P );
    if( embed->containerWinId() == None )
	{
	delete embed;
	return;
	}
    menus.append( embed );
    // in case the app mapped its menu after its mainwindow, check which menu should be shown
    activeWindowChanged( module->activeWindow());
    }

// - if the active window has its topmenu -> show the menu
// - if desktop menu is enabled (i.e. explicitly in kdesktop) :
//   - show it
//   - otherwise show nothing
void Applet::activeWindowChanged( WId w_P )
    {
//    kDebug() << "active:" << w_P << endl;
    for( WId window = w_P;
	 window != None;
	 window = tryTransientFor( window ))
	{
	for( QList< MenuEmbed* >::ConstIterator it = menus.begin();
	     it != menus.end();
	     ++it )
	    {
	    if( window == (*it)->mainWindow())
                {
                activateMenu( *it );
	        return;
		}
	    }
	}
//    kDebug() << "no active" << endl;
    // No menu for active window found - if desktop menu
    // (in kdesktoprc) is enabled, use kdesktop's menu instead of none.
    bool try_desktop = desktop_menu;
    if( !try_desktop && w_P != None )
        { // also use the desktop menu if the active window is desktop
        KWin::WindowInfo info = KWin::windowInfo( w_P, NET::WMWindowType );
        if( info.windowType( SUPPORTED_WINDOW_TYPES ) == NET::Desktop )
            try_desktop = true;
        }
    if( try_desktop )
        {
	for( QList< MenuEmbed* >::ConstIterator it = menus.begin();
	     it != menus.end();
	     ++it )
	    {
            if( (*it)->isDesktopMenu())
                {
                activateMenu( *it );
                return;
                }
            }
        }
    activateMenu( NULL );
    }

void Applet::activateMenu( MenuEmbed* embed_P )
    {
    if( embed_P != active_menu )
	{
//        kDebug() << "activate:" << embed_P << endl;
	if( active_menu != NULL )
	    active_menu->hide();
	active_menu = embed_P;
    if( active_menu != NULL )
    {
	    active_menu->show();
        //if (embed->isDesktopMenu())
        {
            active_menu->setMinimumSize( width(), height() + topEdgeOffset );
        }
    }
    emit updateLayout();
    }
    }

void Applet::updateMenuGeometry( MenuEmbed* embed )
    {
    if( embed == active_menu )
        emit updateLayout();
    }

// If there's no menu for the window, try finding menu for its mainwindow
// (where the window's WM_TRANSIENT_FOR property points).
// If the window is modal (_NET_WM_STATE_MODAL), stop.
WId Applet::tryTransientFor( WId w_P )
    {
    KWin::WindowInfo info = KWin::windowInfo( w_P, NET::WMState );
    if( info.state() & NET::Modal )
	return None;
    WId ret = KWin::transientFor( w_P );
    if( ret == QX11Info::appRootWindow())
        ret = None;
    return ret;
    }

void Applet::menuLost( MenuEmbed* embed )
    {
    for( QList< MenuEmbed* >::Iterator it = menus.begin();
	 it != menus.end();
	 ++it )
	{
	if( *it == embed )
	    {
	    menus.erase( it );
	    embed->deleteLater();
//	    kDebug() << "deleting:" << (*it)->mainWindow() << endl;
	    if( embed == active_menu )
		{
		active_menu = NULL;
		// trigger selecting new active menu
		activeWindowChanged( module->activeWindow());
		}
	    return;
	    }
	}
    }

void Applet::positionChange( Plasma::Position )
    {
    updateTopEdgeOffset();
    }

// Detect mouse movement at the top screen edge also if the menubar
// has a popup open - in such case, Qt has a grab, and this avoids
// Kicker's FittsLawFrame. Therefore move the menubar a bit higher,
// so that it actually is positioned exactly at the screen edge
// (i.e. at a negative y coordinate within this applet, due to
// Kicker's frame).
void Applet::updateTopEdgeOffset()
    {
    QPoint p = topLevelWidget()->mapToGlobal( QPoint( 0, 0 ));
    if( p.y() <= 2 ) // 2 = work also when running in appletproxy
        topEdgeOffset = mapToGlobal( QPoint( 0, 0 )).y() - p.y();
    else
        topEdgeOffset = 0;
    if( active_menu != NULL )
        active_menu->move( active_menu->x(), -topEdgeOffset );
    }

void Applet::paletteChange(const QPalette & /* oldPalette */)
    {
    if( active_menu != NULL )
        {
	active_menu->hide();
	active_menu->show();
	}
    }

void Applet::claimSelection()
    {
    assert( selection == NULL );
    selection = new KSelectionOwner( makeSelectionAtom(), DefaultScreen( QX11Info::display()));
// force taking the selection, but don't kill previous owner
    if( selection->claim( true, false ))
	{
        delete selection_watcher;
        selection_watcher = NULL;
        connect( selection, SIGNAL( lostOwnership()), SLOT( lostSelection()));
        module = new KWinModule;
	connect( module, SIGNAL( windowAdded( WId )), this, SLOT( windowAdded( WId )));
	connect( module, SIGNAL( activeWindowChanged( WId )),
	    this, SLOT( activeWindowChanged( WId )));
	QList< WId > windows = module->windows();
	for( QList< WId >::ConstIterator it = windows.begin();
	     it != windows.end();
	     ++it )
	    windowAdded( *it );
	activeWindowChanged( module->activeWindow());
	}
    else
        lostSelection();
    }

void Applet::lostSelection()
    {
    if( selection == NULL )
        return;
//    kDebug() << "lost selection" << endl;
    for( QList< MenuEmbed* >::ConstIterator it = menus.begin();
	 it != menus.end();
	 ++it )
	delete (*it); // delete all MenuEmbed's = release all menus
    menus.clear();
    active_menu = NULL;
    if( selection_watcher == NULL )
        {
        selection_watcher = new KSelectionWatcher( makeSelectionAtom(), DefaultScreen( QX11Info::display()));
        connect( selection_watcher, SIGNAL( lostOwner()), this, SLOT( claimSelection()));
        }
    delete module;
    module = NULL;
    selection->deleteLater();
    selection = NULL;
    // selection_watcher stays
    }

void Applet::readSettings()
    {
    KConfig cfg( "kdesktoprc", true );
    cfg.setGroup( "Menubar" );
    desktop_menu = cfg.readEntry( "ShowMenubar", false);
    cfg.setGroup( "KDE" );
    if( cfg.readEntry( "macStyle", false) || desktop_menu )
        this->setToolTip("");
    else
        this->setToolTip( i18n(
            "You do not appear to have enabled the standalone menubar; "
            "enable it in the Behavior control module for desktop." ));
    if( !isDisabled() && active_menu == NULL )
        activeWindowChanged( module->activeWindow()); //enforce desktop_menu
    }

void Applet::configure()
    {
    readSettings();
    }

int Applet::widthForHeight( int ) const
    {
    if (active_menu)
        return active_menu->width();
    return 0; // we're stretch applet
    }

int Applet::heightForWidth( int ) const
    {
    // *shrug* running this applet in vertical mode is a bad idea anyway
    return 50;
    }

static Atom selection_atom = None;
static Atom msg_type_atom = None;

static
void initAtoms()
    {
    char nm[ 100 ];
    sprintf( nm, "_KDE_TOPMENU_OWNER_S%d", DefaultScreen( QX11Info::display()));
    char nm2[] = "_KDE_TOPMENU_MINSIZE";
    char* names[ 2 ] = { nm, nm2 };
    Atom atoms[ 2 ];
    XInternAtoms( QX11Info::display(), names, 2, False, atoms );
    selection_atom = atoms[ 0 ];
    msg_type_atom = atoms[ 1 ];
    }

Atom Applet::makeSelectionAtom()
    {
    if( selection_atom == None )
	initAtoms();
    return selection_atom;
    }

MenuEmbed::MenuEmbed( WId mainwindow_P, bool desktop_P,
    QWidget* parent_P, const char* /* name_P*/ )
    :   QX11EmbedWidget( parent_P ),
	main_window( mainwindow_P ),
        desktop( desktop_P )
    {
    //setAutoDelete( false );
    }

void MenuEmbed::windowChanged( WId w_P )
    {
    if( w_P == None )
	static_cast< Applet* >( parent())->menuLost( this );
    }

bool MenuEmbed::x11Event( XEvent* ev_P )
    {
    if( ev_P->type == ConfigureRequest
	&& ev_P->xconfigurerequest.window == containerWinId()
        && ev_P->xconfigurerequest.value_mask & ( CWWidth | CWHeight ))
        {
	XConfigureRequestEvent& ev = ev_P->xconfigurerequest;
        QSize new_size = size();
        if( ev.value_mask & CWWidth )
            new_size.setWidth( ev.width );
        if( ev.value_mask & CWHeight )
            new_size.setHeight( ev.height );
	// resize when the embedded window resizes (still obey min size)
//	kDebug() << "RES:" << embeddedWinId() << ":" << ev.width << ":" << ev.height << endl;
	if( ev.width != width() || ev.height != height())
            {
            resize( ev.width, ev.height );
            static_cast< Applet* >( parent())->updateMenuGeometry( this );
            }
	sendSyntheticConfigureNotifyEvent();
//        int x, y;
//        unsigned int w, h, d, b;
//        Window root;
//        XGetGeometry( QX11Info::display(), embeddedWinId(), &root, &x, &y, &w, &h, &b, &d );
//        kDebug() << "RES3:" << width() << ":" << height() << ":" << w << ":" << h << endl;
	return true;
	}
    return QX11EmbedWidget::x11Event( ev_P );
    }

void MenuEmbed::sendSyntheticConfigureNotifyEvent()
{
    QPoint globalPos = mapToGlobal(QPoint(0,0));
    if (containerWinId()) {
        XConfigureEvent c;
        memset(&c, 0, sizeof(c));
        c.type = ConfigureNotify;
        c.display = QX11Info::display();
        c.send_event = True;
        c.event = containerWinId();
        c.window = winId();
        c.x = globalPos.x();
        c.y = globalPos.y();
        c.width = width();
        c.height = height();
        c.border_width = 0;
        c.above = None;
        c.override_redirect = 0;
        XSendEvent(QX11Info::display(), c.event, true, StructureNotifyMask, (XEvent*)&c);
    }
}

void MenuEmbed::setMinimumSize( int w, int h )
{
    QX11EmbedWidget::setMinimumSize( w, h );
    // tell the menubar also the allowed minimum size
    // the applet won't allow resizing to smaller size
    if( containerWinId() != None )
        {
//        kDebug() << "RES2:" << width() << ":" << height() << ":" << minimumWidth() << ":" << minimumHeight() << endl;
        XEvent ev;
        ev.xclient.display = QX11Info::display();
        ev.xclient.type = ClientMessage;
        ev.xclient.window = containerWinId();
        assert( msg_type_atom != None );
        ev.xclient.message_type = msg_type_atom;
        ev.xclient.format = 32;
        ev.xclient.data.l[0] = QX11Info::appTime();
        ev.xclient.data.l[1] = minimumWidth();
        ev.xclient.data.l[2] = minimumHeight();
        ev.xclient.data.l[3] = 0;
        ev.xclient.data.l[4] = 0;
        XSendEvent( QX11Info::display(), containerWinId(), False, NoEventMask, &ev );
        }
}

} // namespace

#include "menuapplet.moc"
