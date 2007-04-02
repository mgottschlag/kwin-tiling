#include <QToolTip>
#include <QTextStream>
#include <QImage>
#include <QRegExp>
//Added by qt3to4:
#include <QMouseEvent>
#include <QLabel>

#include <kdebug.h>
#include <kapplication.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmenu.h>
#include <k3process.h>
#include <kwm.h>
#include <kconfig.h>
#include <ksystemtrayicon.h>
#include <kconfiggroup.h>

#include <netwm.h>

#include "ksystraycmd.h"
#include "ksystraycmd.moc"
#include <QX11Info>


KSysTrayCmd::KSysTrayCmd()
  : QLabel( 0 ),
    isVisible(true), lazyStart( false ), noquit( false ), quitOnHide( false ), onTop(false), ownIcon(false),
    win(0), client(0), top(0), left(0)
{
  setObjectName("systray_cmd" );
  setAlignment( Qt::AlignCenter );
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
#ifdef __GNUC__
#warning !Qstring == QString.isEmpty ???
#endif	
  if ( command.isEmpty() ) {
      if ( win ) {
	  setTargetWindow( win );
	  return true;
      }

      checkExistingWindows();
      if ( win ) {
        // Window always on top
        if (onTop) {
          KWM::setState(win, NET::StaysOnTop);
        }
        return true;
      }

      errStr = i18n( "No window matching pattern '%1' and no command specified.\n" ,
	    window );
      return false;
  }

  // Run the command and watch for its window
  if ( !startClient() ) {
    errStr = i18n( "KSysTrayCmd: K3ShellProcess cannot find a shell." );
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
  XMapWindow( QX11Info::display(), win );
  // We move the window to the memorized position
  XMoveWindow( QX11Info::display(), win, left, top);

  // Window always on top
  if (onTop)
  {
    KWM::setState(win, NET::StaysOnTop);
  }

  KWM::activateWindow( win );

}

void KSysTrayCmd::hideWindow()
{
  isVisible = false;
  if ( !win )
    return;
  //We memorize the position of the window
  left = KWM::windowInfo(win, NET::WMFrameExtents).frameGeometry().left();
  top=KWM::windowInfo(win, NET::WMFrameExtents).frameGeometry().top();

  XUnmapWindow( QX11Info::display(), win );
}

void KSysTrayCmd::setTargetWindow( WId w )
{
  disconnect( KWM::self(), SIGNAL(windowAdded(WId)), this, SLOT(windowAdded(WId)) );
  connect( KWM::self(), SIGNAL(windowChanged(WId)), SLOT(windowChanged(WId)) );
  win = w;
//  KWM::setSystemTrayWindowFor( winId(), win );
  refresh();
  show();

  if ( isVisible )
    KWM::activateWindow( win );
  else
    hideWindow();

  // Always on top ?
  if (onTop)
  {
    KWM::setState(win, NET::StaysOnTop);
  }
}

//
// Refresh the tray icon
//

void KSysTrayCmd::refresh()
{
//  KWM::setSystemTrayWindowFor( winId(), win ? win : winId() );

  this->setToolTip( QString() );
  if ( win ) {
    KSharedConfig::Ptr appCfg = KGlobal::config();
    KConfigGroup configSaver(appCfg, "System Tray");
    int iconWidth = configSaver.readEntry("systrayIconWidth", 22);

    // ksystraycmd's icon or app's icon
    if (ownIcon)
    {
      // Icefox ### double check after next kdelibs snapshot merge that this does it right
      setWindowIcon( KSystemTrayIcon::loadIcon( qApp->applicationName() ) );
    }
    else
    {
      setWindowIcon( KWM::icon( win, iconWidth, iconWidth, true ) );
    }

    this->setToolTip( KWM::windowInfo( win, NET::WMName ).name() );
  }
  else {
    if ( !tooltip.isEmpty() )
      this->setToolTip( tooltip );
    else if ( !command.isEmpty() )
      this->setToolTip( command );
    else
      this->setToolTip( window );

    // Icefox ### double check after next kdelibs snapshot merge that this does it right
    setIcon( KSystemTrayIcon::loadIcon( qApp->applicationName() ).pixmap() );
  }
}

//
// Client related functions.
//

bool KSysTrayCmd::startClient()
{
  client = new K3ShellProcess();
  *client << command;
  connect( KWM::self(), SIGNAL(windowAdded(WId)), SLOT(windowAdded(WId)) );
  connect( client, SIGNAL( processExited(K3Process *) ),
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
    XMapWindow( QX11Info::display(), win );
    NETRootInfo ri( QX11Info::display(), NET::CloseWindow );
    ri.closeWindowRequest( win );
    win=0;
    noquit = false;

    // We didn't give command, so we didn't open an application.
    // That's why  when the application is closed we aren't informed.
    // So we quit now.
#ifdef __GNUC__
#warning !Qstring == QString.isEmpty ???
#endif    
    if ( command.isEmpty() ) {
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
    KMenu *menu = new KMenu();
    menu->addTitle( *pixmap(), i18n( "KSysTrayCmd" ) );
    QAction * hideShowId = menu->addAction( isVisible ? i18n( "&Hide" ) : i18n( "&Restore" ) );
    QAction * undockId = menu->addAction( QIcon(SmallIcon("close")), i18n( "&Undock" ) );
    QAction * quitId = menu->addAction( QIcon(SmallIcon("application-exit")), i18n( "&Quit" ) );

    QAction * cmd = menu->exec( pos );

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
        NETRootInfo ri( QX11Info::display(), NET::CloseWindow );
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
  QList<WId>::ConstIterator it;
  for ( it = KWM::windows().begin(); it != KWM::windows().end(); ++it ) {
    windowAdded( *it );
    if ( win )
      break;
  }
}

void KSysTrayCmd::windowAdded(WId w)
{
  if ( !window.isEmpty() && ( QRegExp( window ).indexIn( KWM::windowInfo(w,NET::WMName).name() ) == -1 ) )
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
  if ( e->button() == Qt::RightButton )
    execContextMenu( e->globalPos() );
  else if ( lazyStart && ( !hasRunningClient() ) )
  {
    start();
    isVisible=true;
  }
  else if ( quitOnHide && ( hasRunningClient() ) && isVisible )
  {
    NETRootInfo ri( QX11Info::display(), NET::CloseWindow );
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
    static Atom wm_state = XInternAtom( QX11Info::display(), "WM_STATE", False );
    Atom type;
    int format;
    unsigned long nitems, after;
    unsigned char* prop;
    if( XGetWindowProperty( QX11Info::display(), w, wm_state, 0, 0, False, AnyPropertyType,
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
    if( XQueryTree( QX11Info::display(), w, &root, &parent, &children, &nchildren ) != 0 ) {
	for( unsigned int i = 0;
	     i < nchildren && ret == None;
	     ++i )
	    ret = findRealWindow( children[ i ], depth + 1 );
	if( children != NULL )
	    XFree( children );
    }
    return ret;
}
