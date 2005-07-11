#include <qtooltip.h>
#include <qtextstream.h>
#include <qimage.h>
#include <qregexp.h>

#include <kdebug.h>
#include <kapplication.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kpopupmenu.h>
#include <kprocess.h>
#include <kwinmodule.h>
#include <kconfig.h>
#include <ksystemtray.h>

#include <netwm.h>

#include "ksystraycmd.h"
#include "ksystraycmd.moc"


KSysTrayCmd::KSysTrayCmd()
  : QLabel( 0, "systray_cmd" ),
    isVisible(true), lazyStart( false ), noquit( false ), quitOnHide( false ), onTop(false), ownIcon(false),
    win(0), client(0), kwinmodule(0), top(0), left(0)
{
  setAlignment( AlignCenter );
  kwinmodule = new KWinModule( this );
  refresh();
}

KSysTrayCmd::~KSysTrayCmd()
{
    delete client;
}

//
// Main entry point to the class
//

bool KSysTrayCmd::start()
{
  // If we have no command we must catching an existing window
  if ( !command ) {
      if ( win ) {
	  setTargetWindow( win );
	  return true;
      }

      checkExistingWindows();
      if ( win ) {
        // Window always on top
        if (onTop) { 
          KWin::setState(win, NET::StaysOnTop);
        }
        return true;
      }

      errStr = i18n( "No window matching pattern '%1' and no command specified.\n" )
	  .arg( window );
      return false;
  }

  // Run the command and watch for its window
  if ( !startClient() ) {
    errStr = i18n( "KSysTrayCmd: KShellProcess cannot find a shell." );
    clientExited();
    return false;
  }

  return true;
}

//
// Window related functions.
//

void KSysTrayCmd::showWindow()
{
  isVisible = true;
  if ( !win )
    return;
  XMapWindow( qt_xdisplay(), win );
  // We move the window to the memorized position
  XMoveWindow( qt_xdisplay(), win, left, top);  
  
  // Window always on top
  if (onTop)
  {
    KWin::setState(win, NET::StaysOnTop);
  }
  
  KWin::activateWindow( win );
  
}

void KSysTrayCmd::hideWindow()
{
  isVisible = false;
  if ( !win )
    return;
  //We memorize the position of the window
  left = KWin::windowInfo(win).frameGeometry().left();
  top=KWin::windowInfo(win).frameGeometry().top();

  XUnmapWindow( qt_xdisplay(), win );
}

void KSysTrayCmd::setTargetWindow( WId w )
{
  setTargetWindow( KWin::windowInfo( w ) );
}

void KSysTrayCmd::setTargetWindow( const KWin::WindowInfo &info )
{
  disconnect( kwinmodule, SIGNAL(windowAdded(WId)), this, SLOT(windowAdded(WId)) );
  connect( kwinmodule, SIGNAL(windowChanged(WId)), SLOT(windowChanged(WId)) );
  win = info.win();
  KWin::setSystemTrayWindowFor( winId(), win );
  refresh();
  show();

  if ( isVisible )
    KWin::activateWindow( win );
  else
    hideWindow();

  // Always on top ?
  if (onTop)
  {
    KWin::setState(win, NET::StaysOnTop);
  }
}

//
// Refresh the tray icon
//

void KSysTrayCmd::refresh()
{
  KWin::setSystemTrayWindowFor( winId(), win ? win : winId() );

  QToolTip::remove( this );
  if ( win ) {
    KConfig *appCfg = kapp->config();
    KConfigGroupSaver configSaver(appCfg, "System Tray");
    int iconWidth = appCfg->readNumEntry("systrayIconWidth", 22);

    // ksystraycmd's icon or app's icon
    if (ownIcon)
    {
      setPixmap( KSystemTray::loadIcon( kapp->iconName() ) );
    }
    else
    {
      setPixmap( KWin::icon( win, iconWidth, iconWidth, true ) );
    }

    QToolTip::add( this, KWin::windowInfo( win ).name() );
  }
  else {
    if ( !tooltip.isEmpty() )
      QToolTip::add( this, tooltip );
    else if ( !command.isEmpty() )
      QToolTip::add( this, command );
    else
      QToolTip::add( this, window );

    setPixmap( KSystemTray::loadIcon( kapp->iconName() ) );
  }
}

//
// Client related functions.
//

bool KSysTrayCmd::startClient()
{
  client = new KShellProcess();
  *client << command;
  connect( kwinmodule, SIGNAL(windowAdded(WId)), SLOT(windowAdded(WId)) );
  connect( client, SIGNAL( processExited(KProcess *) ),
	   this, SLOT( clientExited() ) );

  return client->start();
}

void KSysTrayCmd::clientExited()
{
  delete client;
  client = 0;
  win = 0;

  if ( lazyStart && noquit )
    refresh();
  else
    qApp->quit();
}

void KSysTrayCmd::quitClient()
{
  if ( win ) {
    // Before sending the close request we have to show the window
    XMapWindow( qt_xdisplay(), win );
    NETRootInfo ri( qt_xdisplay(), NET::CloseWindow );
    ri.closeWindowRequest( win );
    win=0;
    noquit = false;

    // We didn't give command, so we didn't open an application.
    // That's why  when the application is closed we aren't informed.
    // So we quit now.
    if ( !command ) {
      qApp->quit();
    }
  }
  else {
    qApp->quit();
  }
}

void KSysTrayCmd::quit()
{
    if ( !isVisible ) {
	showWindow();
    }
    qApp->quit();
}

void KSysTrayCmd::execContextMenu( const QPoint &pos )
{
    KPopupMenu *menu = new KPopupMenu();
    menu->insertTitle( *pixmap(), i18n( "KSysTrayCmd" ) );
    int hideShowId = menu->insertItem( isVisible ? i18n( "&Hide" ) : i18n( "&Restore" ) );
    int undockId = menu->insertItem( SmallIcon("close"), i18n( "&Undock" ) );
    int quitId = menu->insertItem( SmallIcon("exit"), i18n( "&Quit" ) );

    int cmd = menu->exec( pos );

    if ( cmd == quitId )
      quitClient();
    else if ( cmd == undockId )
      quit();
    else if ( cmd == hideShowId )
    {
      if ( lazyStart && ( !hasRunningClient() ) )
      {
        start();
        isVisible=true;
      }
      else if ( quitOnHide && ( hasRunningClient() ) && isVisible )
      {
        NETRootInfo ri( qt_xdisplay(), NET::CloseWindow );
        ri.closeWindowRequest( win );
        isVisible=false;
      }
      else
        toggleWindow();
    }

    delete menu;
}

void KSysTrayCmd::checkExistingWindows()
{
  QValueList<WId>::ConstIterator it;
  for ( it = kwinmodule->windows().begin(); it != kwinmodule->windows().end(); ++it ) {
    windowAdded( *it );
    if ( win )
      break;
  }
}

void KSysTrayCmd::windowAdded(WId w)
{
  if ( !window.isEmpty() && ( QRegExp( window ).search( KWin::windowInfo(w).name() ) == -1 ) )
    return; // no match
  setTargetWindow( w );
}

void KSysTrayCmd::windowChanged( WId w )
{
  if ( w != win )
    return;
  refresh();
}

//
// Tray icon event handlers
//

void KSysTrayCmd::mousePressEvent( QMouseEvent *e )
{
  if ( e->button() == RightButton )
    execContextMenu( e->globalPos() );
  else if ( lazyStart && ( !hasRunningClient() ) )
  {
    start();
    isVisible=true;
  }
  else if ( quitOnHide && ( hasRunningClient() ) && isVisible )
  {
    NETRootInfo ri( qt_xdisplay(), NET::CloseWindow );
    ri.closeWindowRequest( win );
    isVisible=false;
  }
  else
    toggleWindow();
}

WId KSysTrayCmd::findRealWindow( WId w, int depth )
{
    if( depth > 5 )
	return None;
    static Atom wm_state = XInternAtom( qt_xdisplay(), "WM_STATE", False );
    Atom type;
    int format;
    unsigned long nitems, after;
    unsigned char* prop;
    if( XGetWindowProperty( qt_xdisplay(), w, wm_state, 0, 0, False, AnyPropertyType,
	&type, &format, &nitems, &after, &prop ) == Success ) {
	if( prop != NULL )
	    XFree( prop );
	if( type != None )
	    return w;
    }
    Window root, parent;
    Window* children;
    unsigned int nchildren;
    Window ret = None;
    if( XQueryTree( qt_xdisplay(), w, &root, &parent, &children, &nchildren ) != 0 ) {
	for( unsigned int i = 0;
	     i < nchildren && ret == None;
	     ++i )
	    ret = findRealWindow( children[ i ], depth + 1 );
	if( children != NULL )
	    XFree( children );
    }
    return ret;
}
