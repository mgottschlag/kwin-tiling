#include <qtooltip.h>
#include <qtextstream.h>
#include <qimage.h>
#include <qregexp.h>

#include <kapplication.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kpopupmenu.h>
#include <kprocess.h>
#include <kwinmodule.h>

#include <netwm.h>

#include <X11/Xlib.h>
const int XFocusOut = FocusOut;
const int XFocusIn = FocusIn;
#undef FocusOut
#undef FocusIn
#undef KeyPress
#undef KeyRelease

extern Time qt_x_time;


#include "ksystraycmd.h"
#include "ksystraycmd.moc"


KSysTrayCmd::KSysTrayCmd()
  : QLabel( 0, "systray_cmd" ),
    isVisible(true), lazyStart( false ), noquit( false ), quitOnHide( false ),
    win(0), client(0), kwinmodule(0)
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
  if ( !window.isEmpty() )
    kwinmodule->doNotManage( window );

  // If we have no command we must catching an existing window
  if ( !command ) {
      if ( win ) {
	  setTargetWindow( win );
	  return true;
      }

      checkExistingWindows();
      if ( win )
	  return true;

      errStr = i18n( "No window matching pattern '%1' and no command specified.\n" )
	  .arg( window );
      return false;
  }

  // Run the command and watch for its window
  if ( !startClient() ) {
    errStr = i18n( "KSysTrayCmd: KShellProcess can't find a shell." );
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
  KWin::setActiveWindow( win );
}

void KSysTrayCmd::hideWindow()
{
  isVisible = false;
  if ( !win )
    return;
  XUnmapWindow( qt_xdisplay(), win );
}

void KSysTrayCmd::setTargetWindow( WId w )
{
  setTargetWindow( KWin::info( w ) );
}

void KSysTrayCmd::setTargetWindow( const KWin::Info &info )
{
  disconnect( kwinmodule, SIGNAL(windowAdded(WId)), this, SLOT(windowAdded(WId)) );
  connect( kwinmodule, SIGNAL(windowChanged(WId)), SLOT(windowChanged(WId)) );
  win = info.win;
  KWin::setSystemTrayWindowFor( winId(), win );
  refresh();
  show();

  if ( isVisible )
    KWin::setActiveWindow( win );
  else
    hideWindow();
}

//
// Refresh the tray icon
//

void KSysTrayCmd::refresh()
{
  KWin::setSystemTrayWindowFor( winId(), win ? win : winId() );

  if ( win ) {
    setPixmap( KWin::icon( win, 20, 20, true ) );
    QToolTip::add( this, KWin::info( win ).name );
  }
  else {
    if ( !tooltip.isEmpty() )
      QToolTip::add( this, tooltip );
    else if ( !command.isEmpty() )
      QToolTip::add( this, command );
    else
      QToolTip::add( this, window );

    QImage img;
    img = kapp->icon();
    img = img.smoothScale( 20, 20 );
    QPixmap pix;
    pix = img;
    setPixmap( pix );
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
    NETRootInfo ri( qt_xdisplay(), NET::CloseWindow );
    ri.closeWindowRequest( win );
    noquit = false;
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
      toggleWindow();

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
  if ( !window.isEmpty() && ( QRegExp( window ).search( KWin::info(w).name ) == -1 ) )
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
    start();
  else if ( quitOnHide && ( hasRunningClient() ) && isVisible )
  {
    NETRootInfo ri( qt_xdisplay(), NET::CloseWindow );
    ri.closeWindowRequest( win );
  }
  else
    toggleWindow();
}

void KSysTrayCmd::enterEvent( QEvent * )
{
  // Fake a focus event when the mouse points at our tray icon. This is
  // needed for our tooltips to work.
  if ( !qApp->focusWidget() ) {
    XEvent ev;
    memset(&ev, 0, sizeof(ev));
    ev.xfocus.display = qt_xdisplay();
    ev.xfocus.type = XFocusIn;
    ev.xfocus.window = winId();
    ev.xfocus.mode = NotifyNormal;
    ev.xfocus.detail = NotifyAncestor;
    Time time = qt_x_time;
    qt_x_time = 1;
    qApp->x11ProcessEvent( &ev );
	qt_x_time = time;
  }
}


