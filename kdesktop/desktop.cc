/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/


#include "desktop.h"
#include "krootwm.h"
#include "bgmanager.h"
#include "bgsettings.h"
#include "startupid.h"
#include "kdiconview.h"
#include "minicli.h"
#include "lockeng.h"
#include "kdesktopsettings.h"
#include "klaunchsettings.h"
#include "kdesktopadaptor.h"

#include <string.h>
#include <unistd.h>
#include <k3colordrag.h>

#include <QDir>
#include <QEvent>

#include <netwm.h>
#include <kaction.h>
#include <kcursor.h>
#include <kdebug.h>
#include <kstandarddirs.h>
#include <kimageio.h>
#include <kinputdialog.h>
#include <kipc.h>
#include <klocale.h>
#include <kio/netaccess.h>
#include <kprocess.h>
#include <ksycoca.h>
#include <ktempfile.h>
#include <kmessagebox.h>
#include <kglobalaccel.h>
#include <kauthorized.h>
#include <kwinmodule.h>
#include <krun.h>
#include <kwin.h>
#include <kglobalsettings.h>
#include <kmenu.h>
#include <kapplication.h>
#include <QX11Info>
#include <QDesktopWidget>

KRootWidget::KRootWidget() : QObject()
{
     kapp->desktop()->installEventFilter(this);
     kapp->desktop()->setAcceptDrops( true );
}

bool KRootWidget::eventFilter ( QObject *, QEvent * e )
{
     if (e->type() == QEvent::MouseButtonPress)
     {
       QMouseEvent *me = static_cast<QMouseEvent *>(e);
       KRootWm::self()->mousePressed( me->globalPos(), me->button() );
       return true;
     }
     else if (e->type() == QEvent::Wheel)
     {
       QWheelEvent *we = static_cast<QWheelEvent *>(e);
       emit wheelRolled(we->delta());
       return true;
     }
     else if ( e->type() == QEvent::DragEnter )
     {
       QDragEnterEvent* de = static_cast<QDragEnterEvent *>( e );
       bool b = !KGlobal::config()->isImmutable() && !KGlobal::dirs()->isRestrictedResource( "wallpaper" );

       bool imageURL = false;
       if ( KUrl::List::canDecode( de->mimeData() ) )
       {
         KUrl url = KUrl::List::fromMimeData( de->mimeData() ).first();
         KMimeType::Ptr mime = KMimeType::findByURL( url );
         if ( mime && ( KImageIO::isSupported( mime->name(), KImageIO::Reading ) || mime->is( "image/svg+xml" ) ) )
           imageURL = true;
       }

       b = b && ( K3ColorDrag::canDecode( de ) || Q3ImageDrag::canDecode( de ) || imageURL );
       de->setAccepted( b );
       return true;
     }
     else if ( e->type() == QEvent::Drop )
     {
       QDropEvent* de = static_cast<QDropEvent*>( e );
       if ( K3ColorDrag::canDecode( de ) )
         emit colorDropEvent( de );
       else if ( Q3ImageDrag::canDecode( de ) )
         emit imageDropEvent( de );
       else if ( KUrl::List::canDecode( de->mimeData() ) ) {
         KUrl url = KUrl::List::fromMimeData( de->mimeData() ).first();
         emit newWallpaper( url );
       }
       return true;
     }
     return false; // Don't filter.
}

// -----------------------------------------------------------------------------
#define DEFAULT_DELETEACTION 1

// for multihead - from main.cc
extern int kdesktop_screen_number;

KDesktop::WheelDirection KDesktop::m_eWheelDirection = KDesktop::m_eDefaultWheelDirection;
const char* KDesktop::m_wheelDirectionStrings[2] = { "Forward", "Reverse" };

KDesktop::KDesktop( bool x_root_hack, bool wait_for_kded ) :
    QWidget( 0L, Qt::WResizeNoErase | ( x_root_hack ? (Qt::WStyle_NoBorder) : ( Qt::WFlags )0 ) ),
    // those two WStyle_ break kdesktop when the root-hack isn't used (no Dnd)
   startup_id( NULL )
{
  setObjectName( "desktop" );
  m_bWaitForKded = wait_for_kded;
  m_miniCli = 0; // created on demand
  m_actionCollection = 0; // created later
  KGlobal::locale()->insertCatalog("kdesktop");
  KGlobal::locale()->insertCatalog("libkonq"); // needed for apps using libkonq
  KGlobal::locale()->insertCatalog("libdmctl");

  setWindowTitle( "KDE Desktop" );

  (void) new KDesktopAdaptor( this );
  QDBus::sessionBus().registerObject("/Desktop", this);

  setAcceptDrops(true); // WStyle_Customize seems to disable that
  m_pKwinmodule = new KWinModule( this );
  updateWorkAreaTimer = new QTimer( this );
  updateWorkAreaTimer->setSingleShot( true );

  connect( updateWorkAreaTimer, SIGNAL( timeout() ),
           this, SLOT( updateWorkArea() ) );
  connect( m_pKwinmodule, SIGNAL( workAreaChanged() ),
           this, SLOT( workAreaChanged() ) );

  // Dont repaint on configuration changes during construction
  m_bInit = true;

  // It's the child widget that gets the focus, not us
  setFocusPolicy( Qt::NoFocus );

  if ( x_root_hack )
  {
    // this is a ugly hack to make Dnd work
    // Matthias told me that it won't be necessary with kwin
    // actually my first try with ICCCM (Dirk) :-)
    unsigned long data[2];
    data[0] = (unsigned long) 1;
    data[1] = (unsigned long) 0; // None; (Werner)
    Atom wm_state = XInternAtom(QX11Info::display(), "WM_STATE", False);
    XChangeProperty(QX11Info::display(), winId(), wm_state, wm_state, 32,
                    PropModeReplace, (unsigned char *)data, 2);

  }

  setGeometry( QApplication::desktop()->geometry() );
  lower();

  connect( qApp, SIGNAL( aboutToQuit() ),
           this, SLOT( slotShutdown() ) );

  connect(kapp, SIGNAL(settingsChanged(int)),
          this, SLOT(slotSettingsChanged(int)));
  kapp->addKipcEventMask(KIPC::SettingsChanged);

  kapp->addKipcEventMask(KIPC::IconChanged);
  connect(kapp, SIGNAL(iconChanged(int)), this, SLOT(slotIconChanged(int)));

  connect(KSycoca::self(), SIGNAL(databaseChanged()),
          this, SLOT(slotDatabaseChanged()));

  m_pIconView = 0;
  m_pRootWidget = 0;
  bgMgr = 0;
  initRoot();

  QTimer::singleShot(0, this, SLOT( slotStart() ));

  connect( kapp->desktop(), SIGNAL( resized( int )), SLOT( desktopResized()));
}

void
KDesktop::initRoot()
{
  Display *dpy = QX11Info::display();
  Window root = RootWindow(dpy, kdesktop_screen_number);
  XDefineCursor(dpy, root, cursor().handle());

  m_bDesktopEnabled = KDesktopSettings::desktopEnabled();
  if ( !m_bDesktopEnabled && !m_pRootWidget )
  {
     hide();
     delete bgMgr;
     bgMgr = 0;
     if ( m_pIconView )
        m_pIconView->saveIconPositions();
     delete m_pIconView;
     m_pIconView = 0;

     XWindowAttributes attrs;
     XGetWindowAttributes(dpy, root, &attrs);
     XSelectInput(dpy, root, attrs.your_event_mask | ButtonPressMask);

     m_pRootWidget = new KRootWidget;
     connect(m_pRootWidget, SIGNAL(wheelRolled(int)), this, SLOT(slotSwitchDesktops(int)));
     connect(m_pRootWidget, SIGNAL(colorDropEvent(QDropEvent*)), this, SLOT(handleColorDropEvent(QDropEvent*)) );
     connect(m_pRootWidget, SIGNAL(imageDropEvent(QDropEvent*)), this, SLOT(handleImageDropEvent(QDropEvent*)) );
     connect(m_pRootWidget, SIGNAL(newWallpaper(const KUrl&)), this, SLOT(slotNewWallpaper(const KUrl&)) );

     // Geert Jansen: backgroundmanager belongs here
     // TODO tell KBackgroundManager if we change widget()
     bgMgr = new KBackgroundManager( m_pIconView, m_pKwinmodule );
     bgMgr->setExport(1);
     connect( bgMgr, SIGNAL( initDone()), SLOT( backgroundInitDone()));
     if (!m_bInit)
     {
        delete KRootWm::self();
        KRootWm* krootwm = new KRootWm( this ); // handler for root menu (used by kdesktop on RMB click)
        connect(m_actionCollection->action("Lock Session"), SIGNAL(triggered(bool)), krootwm, SLOT(slotLock()));
     }
  }
  else if (m_bDesktopEnabled && !m_pIconView)
  {
     delete bgMgr;
     bgMgr = 0;
     delete m_pRootWidget;
     m_pRootWidget = 0;
     m_pIconView = new KDIconView( this, 0 );
     connect( m_pIconView, SIGNAL( imageDropEvent( QDropEvent * ) ),
              this, SLOT( handleImageDropEvent( QDropEvent * ) ) );
     connect( m_pIconView, SIGNAL( colorDropEvent( QDropEvent * ) ),
              this, SLOT( handleColorDropEvent( QDropEvent * ) ) );
     connect( m_pIconView, SIGNAL( newWallpaper( const KUrl & ) ),
              this, SLOT( slotNewWallpaper( const KUrl & ) ) );
     connect( m_pIconView, SIGNAL( wheelRolled( int ) ),
              this, SLOT( slotSwitchDesktops( int ) ) );

     // All the QScrollView/QWidget-specific stuff should go here, so that we can use
     // another qscrollview/widget instead of the iconview and use the same code
     m_pIconView->setVScrollBarMode( Q3ScrollView::AlwaysOff );
     m_pIconView->setHScrollBarMode( Q3ScrollView::AlwaysOff );
     m_pIconView->setDragAutoScroll( false );
     m_pIconView->setFrameStyle( QFrame::NoFrame );
     // This is the replacement for X11Relative
     m_pIconView->viewport()->setBackgroundRole( QPalette::NoRole );
     m_pIconView->viewport()->setForegroundRole( QPalette::NoRole );
     m_pIconView->setFocusPolicy( Qt::StrongFocus );
     m_pIconView->viewport()->setFocusPolicy( Qt::StrongFocus );
     m_pIconView->setGeometry( geometry() );
     m_pIconView->show();

     // Geert Jansen: backgroundmanager belongs here
     // TODO tell KBackgroundManager if we change widget()
     bgMgr = new KBackgroundManager( m_pIconView, m_pKwinmodule );
     bgMgr->setExport(1);
     connect( bgMgr, SIGNAL( initDone()), SLOT( backgroundInitDone()));

     //kDebug(1204) << "KDesktop constructor -> workAreaChanged" << endl;
     workAreaChanged();
     if (!m_bInit)
     {
        m_pIconView->initConfig( m_bInit );
        m_pIconView->start();
        delete KRootWm::self();
        KRootWm* krootwm = new KRootWm( this ); // handler for root menu (used by kdesktop on RMB click)
        connect(m_actionCollection->action("Lock Session"), SIGNAL(triggered(bool)), krootwm, SLOT(slotLock()));
     }
   } else {
      QDBusInterfacePtr ksmserver( "org.kde.ksmserver", "/KSMServer", "org.kde.KSMServerInterface" );
      if ( ksmserver->isValid() )
          ksmserver->call( "resumeStartup", QString( "kdesktop" ) );
   }

   KWin::setType( winId(), NET::Desktop );
   KWin::setState( winId(), NET::SkipPager );
   KWin::setOnAllDesktops( winId(), true );
}

void
KDesktop::backgroundInitDone()
{
    //kDebug(1204) << "KDesktop::backgroundInitDone" << endl;
    // avoid flicker
    if (m_bDesktopEnabled)
    {
#warning backgroundPixmap in Qt4 is a noop
        /*
       const QPixmap *bg = QApplication::desktop()->screen()->backgroundPixmap();
       if ( bg )
          m_pIconView->setErasePixmap( *bg );
        */
       show();
       kapp->sendPostedEvents();
    }

    QDBusInterfacePtr ksmserver( "org.kde.ksmserver", "/KSMServer", "org.kde.KSMServerInterface" );
    if ( ksmserver->isValid() )
        ksmserver->call( "suspendStartup", QString( "kdesktop" ) );
}

void
KDesktop::slotStart()
{
  //kDebug(1204) << "KDesktop::slotStart" << endl;
  if (!m_bInit) return;

  initConfig();

//   if (m_bDesktopEnabled)
//   {
//      // We need to be visible in order to insert icons, even if the background isn't ready yet...

//      show();
//   }

  // Now we may react to configuration changes
  m_bInit = false;

  if (m_pIconView)
     m_pIconView->start();

  // Global keys
  KActionCollection* actionCollection = m_actionCollection = new KActionCollection( this );
  (void) new KRootWm( this );
  KAction* a = 0L;

#include "kdesktopbindings.cpp"

  m_actionCollection->readSettings();

  connect(kapp, SIGNAL(appearanceChanged()), SLOT(slotConfigure()));

  QTimer::singleShot(300, this, SLOT( slotUpAndRunning() ));
}

void
KDesktop::runAutoStart()
{
     // now let's execute all the stuff in the autostart folder.
     // the stuff will actually be really executed when the event loop is
     // entered, since KRun internally uses a QTimer
     QDir dir( KGlobalSettings::autostartPath() );
     QStringList entries = dir.entryList( QDir::Files );
     QStringList::Iterator it = entries.begin();
     QStringList::Iterator end = entries.end();
     for (; it != end; ++it )
     {
            // Don't execute backup files
            if ( (*it).right(1) != "~" && (*it).right(4) != ".bak" &&
                 ( (*it)[0] != '%' || (*it).right(1) != "%" ) &&
                 ( (*it)[0] != '#' || (*it).right(1) != "#" ) )
            {
                KUrl url;
                url.setPath( dir.absolutePath() + '/' + (*it) );
                (void) new KRun( url, 0, true );
            }
     }
}

// -----------------------------------------------------------------------------

KDesktop::~KDesktop()
{
  delete m_miniCli;
  m_miniCli = 0; // see #120382
  delete bgMgr;
  bgMgr = 0;
  delete startup_id;
}

// -----------------------------------------------------------------------------

void KDesktop::initConfig()
{
    if (m_pIconView)
       m_pIconView->initConfig( m_bInit );

    if ( m_actionCollection )
    {
        m_actionCollection->readSettings();
    }

    KLaunchSettings::self()->readConfig();
    if( !KLaunchSettings::busyCursor() )
    {
        delete startup_id;
        startup_id = NULL;
    }
    else
    {
        if( startup_id == NULL )
            startup_id = new StartupId;
        startup_id->configure();
    }

    set_vroot = KDesktopSettings::setVRoot();
    slotSetVRoot(); // start timer

    m_bWheelSwitchesWorkspace = KDesktopSettings::wheelSwitchesWorkspace();

    const char* forward_string = m_wheelDirectionStrings[Forward];
    m_eWheelDirection =
        (KDesktopSettings::wheelDirection() == forward_string) ? Forward : Reverse;
}

// -----------------------------------------------------------------------------

void KDesktop::slotExecuteCommand()
{
    popupExecuteCommand( QString() );
}

void KDesktop::popupExecuteCommand( const QString& command )
{
  if (m_bInit)
      return;

  if (!KAuthorized::authorizeKAction("run_command"))
      return;

  // Created on demand
  if ( !m_miniCli )
  {
      m_miniCli = new Minicli;
      m_miniCli->adjustSize(); // for the centering below
  }

  if (!command.isEmpty())
      m_miniCli->setCommand(command);

  // Move minicli to the current desktop
  NETWinInfo info( QX11Info::display(), m_miniCli->winId(), QX11Info::appRootWindow(), NET::WMDesktop );
  int currentDesktop = kwinModule()->currentDesktop();
  if ( info.desktop() != currentDesktop )
      info.setDesktop( currentDesktop );

  if ( m_miniCli->isVisible() ) {
      KWin::forceActiveWindow( m_miniCli->winId() );
  } else {
      QRect rect = KGlobalSettings::desktopGeometry(QCursor::pos());
      m_miniCli->move(rect.x() + (rect.width() - m_miniCli->width())/2,
                      rect.y() + (rect.height() - m_miniCli->height())/2);
      m_miniCli->show(); // non-modal
  }
}

void KDesktop::slotSwitchUser()
{
     KRootWm::self()->slotSwitchUser();
}

void KDesktop::slotShowWindowList()
{
     KRootWm::self()->slotWindowList();
}

void KDesktop::slotShowTaskManager()
{
    //kDebug(1204) << "Launching KSysGuard..." << endl;
    KProcess* p = new KProcess;
    Q_CHECK_PTR(p);

    *p << "ksysguard";
    *p << "--showprocesses";

    p->start(KProcess::DontCare);

    delete p;
}

// -----------------------------------------------------------------------------

void KDesktop::rearrangeIcons()
{
    if (m_pIconView)
        m_pIconView->rearrangeIcons();
}

void KDesktop::lineupIcons()
{
    if (m_pIconView)
        m_pIconView->lineupIcons();
}

void KDesktop::selectAll()
{
    if (m_pIconView)
        m_pIconView->selectAll( true );
}

void KDesktop::unselectAll()
{
    if (m_pIconView)
        m_pIconView->selectAll( false );
}

QStringList KDesktop::selectedURLs()
{
    if (m_pIconView)
        return m_pIconView->selectedURLs();
    return QStringList();
}

void KDesktop::refreshIcons()
{
    if (m_pIconView)
        m_pIconView->refreshIcons();
}

KActionCollection * KDesktop::actionCollection()
{
    if (!m_pIconView)
       return 0;
    return m_pIconView->actionCollection();
}

KUrl KDesktop::url() const
{
    if (m_pIconView)
        return m_pIconView->url();
    return KUrl();
}

// -----------------------------------------------------------------------------

void KDesktop::slotConfigure()
{
    configure();
}

void KDesktop::configure()
{
    // re-read configuration and apply it
    KGlobal::config()->reparseConfiguration();
    KDesktopSettings::self()->readConfig();

    // If we have done start() already, then re-configure.
    // Otherwise, start() will call initConfig anyway
    if (!m_bInit)
    {
       initRoot();
       initConfig();
       KRootWm::self()->initConfig();
    }

    if (m_actionCollection)
    {
       m_actionCollection->readSettings();
    }
}

void KDesktop::slotSettingsChanged(int category)
{
    //kDebug(1204) << "KDesktop::slotSettingsChanged" << endl;
    if (category == KApplication::SETTINGS_PATHS)
    {
        kDebug(1204) << "KDesktop::slotSettingsChanged SETTINGS_PATHS" << endl;
        if (m_pIconView)
            m_pIconView->recheckDesktopURL();
    }
    else if (category == KApplication::SETTINGS_SHORTCUTS)
    {
        kDebug(1204) << "KDesktop::slotSettingsChanged SETTINGS_SHORTCUTS" << endl;
        m_actionCollection->readSettings();
    }
}

void KDesktop::slotIconChanged(int group)
{
    if ( group == K3Icon::Desktop )
    {
        kDebug(1204) << "KDesktop::slotIconChanged" << endl;
        refresh();
    }
}

void KDesktop::slotDatabaseChanged()
{
    //kDebug(1204) << "KDesktop::slotDatabaseChanged" << endl;
    if (m_bInit) // kded is done, now we can "start" for real
        slotStart();
    if (m_pIconView && KSycoca::isChanged("mimetypes"))
        m_pIconView->refreshMimeTypes();
}

void KDesktop::refresh()
{
  // George Staikos 3/14/01
  // This bit will just refresh the desktop and icons.  Now I have code
  // in KWin to do a complete refresh so this isn't really needed.
  // I'll leave it in here incase the plan is changed again
#if 0
  m_bNeedRepaint |= 1;
  updateWorkArea();
#endif
  QDBusInterfacePtr kwin( "org.kde.kwin", "/KWin", "org.kde.KWin" );
  if ( kwin->isValid() )
      kwin->call( "refresh" );
  refreshIcons();
}

// -----------------------------------------------------------------------------

void KDesktop::slotSetVRoot()
{
    if (!m_pIconView)
        return;

    if (KWin::windowInfo(winId()).mappingState() == NET::Withdrawn) {
        QTimer::singleShot(100, this, SLOT(slotSetVRoot()));
        return;
    }

	QX11Info info;
    unsigned long rw = RootWindowOfScreen(ScreenOfDisplay(QX11Info::display(), info.screen()));
    unsigned long vroot_data[1] = { m_pIconView->viewport()->winId() };
    static Atom vroot = XInternAtom(QX11Info::display(), "__SWM_VROOT", False);

    Window rootReturn, parentReturn, *children;
    unsigned int numChildren;
    Window top = winId();
    while (1) {
        /*int ret = */XQueryTree(QX11Info::display(), top , &rootReturn, &parentReturn,
                                 &children, &numChildren);
        if (children)
            XFree((char *)children);
        if (parentReturn == rw) {
            break;
        } else
            top = parentReturn;
    }
    if ( set_vroot )
        XChangeProperty(QX11Info::display(), top, vroot, XA_WINDOW, 32,
                        PropModeReplace, (unsigned char *)vroot_data, 1);
    else
        XDeleteProperty (QX11Info::display(), top, vroot);
}

// -----------------------------------------------------------------------------

void KDesktop::slotShutdown()
{
    if ( m_pIconView )
        m_pIconView->saveIconPositions();
    if ( m_miniCli )
        m_miniCli->saveConfig();
}

// don't hide when someone presses Alt-F4 on us
void KDesktop::closeEvent(QCloseEvent *e)
{
    e->ignore();
}

void KDesktop::workAreaChanged()
{
    //kDebug(1204) << "KDesktop::workAreaChanged() -> starting timer" << endl;
    updateWorkAreaTimer->stop();
    updateWorkAreaTimer->start( 100 );
}

void KDesktop::updateWorkArea()
{
    if (m_pIconView)
    {
        QRect wr( kwinModule()->workArea( kwinModule()->currentDesktop() ) );
        m_pIconView->updateWorkArea( wr );
    }
}

void KDesktop::slotSwitchDesktops(int delta)
{
    if(m_bWheelSwitchesWorkspace && KWin::numberOfDesktops() > 1)
    {
      int newDesk, curDesk = KWin::currentDesktop();

      if( (delta < 0 && m_eWheelDirection == Forward) || (delta > 0 && m_eWheelDirection == Reverse) )
        newDesk = curDesk % KWin::numberOfDesktops() + 1;
      else
        newDesk = ( KWin::numberOfDesktops() + curDesk - 2 ) % KWin::numberOfDesktops() + 1;

      KWin::setCurrentDesktop( newDesk );
    }
}

void KDesktop::handleColorDropEvent(QDropEvent * e)
{
    KMenu popup;
    QAction* primary = popup.addAction(SmallIconSet("colors"), i18n("Set as Primary Background Color"));
    popup.addAction(SmallIconSet("colors"), i18n("Set as Secondary Background Color"));
    QAction* result = popup.exec(e->pos());

    if (result != 0) {
      QColor c;
      K3ColorDrag::decode(e, c);
      bgMgr->setColor(c, result == primary);
      bgMgr->setWallpaper(0, 0);
    }
}

void KDesktop::handleImageDropEvent(QDropEvent * e)
{
    KMenu popup;
    QAction* saveToDesktop  = 0;
    QAction* setAsWallpaper = 0;
    if ( m_pIconView )
       saveToDesktop = popup.addAction(SmallIconSet("filesave"), i18n("&Save to Desktop..."));
    if ( ( m_pIconView && m_pIconView->maySetWallpaper() ) || m_pRootWidget )
       setAsWallpaper = popup.addAction(SmallIconSet("background"), i18n("Set as &Wallpaper"));
    popup.addSeparator();
    popup.addAction(SmallIconSet("cancel"), i18n("&Cancel"));
    QAction* result = popup.exec(e->pos());

    if (result == saveToDesktop)
    {
        bool ok = true;
        QString filename = KInputDialog::getText(QString(), i18n("Enter a name for the image below:"), QString(), &ok, m_pIconView);

        if (!ok)
        {
            return;
        }

        if (filename.isEmpty())
        {
            filename = i18n("image.png");
        }
        else if (filename.right(4).toLower() != ".png")
        {
            filename += ".png";
        }

        QImage i;
        Q3ImageDrag::decode(e, i);
        KTempFile tmpFile(QString::null, filename);
        i.save(tmpFile.name(), "PNG");
        // We pass 0 as parent window because passing the desktop is not a good idea
        KUrl src;
        src.setPath( tmpFile.name() );
        KUrl dest( KDIconView::desktopURL() );
        dest.addPath( filename );
        KIO::NetAccess::copy( src, dest, 0 );
        tmpFile.unlink();
    }
    else if (result == setAsWallpaper)
    {
        QImage i;
        Q3ImageDrag::decode(e, i);
        KTempFile tmpFile(KGlobal::dirs()->saveLocation("wallpaper"), ".png");
        i.save(tmpFile.name(), "PNG");
        kDebug(1204) << "KDesktop::contentsDropEvent " << tmpFile.name() << endl;
        bgMgr->setWallpaper(tmpFile.name());
    }
}

void KDesktop::slotNewWallpaper(const KUrl &url)
{
    // This is called when a file containing an image is dropped
    // (called by KonqOperations)
    if ( url.isLocalFile() )
        bgMgr->setWallpaper( url.path() );
    else
    {
        // Figure out extension
        QString fileName = url.fileName();
        QFileInfo fileInfo( fileName );
        QString ext = fileInfo.suffix();
        // Store tempfile in a place where it will still be available after a reboot
        KTempFile tmpFile( KGlobal::dirs()->saveLocation("wallpaper"), '.' + ext );
        KUrl localURL; localURL.setPath( tmpFile.name() );
        // We pass 0 as parent window because passing the desktop is not a good idea
        KIO::NetAccess::file_copy( url, localURL, -1, true /*overwrite*/ );
        bgMgr->setWallpaper( localURL.path() );
    }
}

// For the dbus interface [maybe the dbus interface should have those extra args?]
void KDesktop::logout()
{
    logout( KWorkSpace::ShutdownConfirmDefault,
            KWorkSpace::ShutdownTypeNone );
}

void KDesktop::logout( KWorkSpace::ShutdownConfirm confirm,
                       KWorkSpace::ShutdownType sdtype )
{
    if( !KWorkSpace::requestShutDown( confirm, sdtype ) )
        // this i18n string is also in kicker/applets/run/runapplet
        KMessageBox::error( this, i18n("Could not log out properly.\nThe session manager cannot "
                                        "be contacted. You can try to force a shutdown by pressing "
                                        "Ctrl+Alt+Backspace; note, however, that your current session "
                                        "will not be saved with a forced shutdown." ) );
}

void KDesktop::slotLogout()
{
    logout( KWorkSpace::ShutdownConfirmDefault,
            KWorkSpace::ShutdownTypeDefault );
}

void KDesktop::slotLogoutNoCnf()
{
    logout( KWorkSpace::ShutdownConfirmNo,
            KWorkSpace::ShutdownTypeNone );
}

void KDesktop::slotHaltNoCnf()
{
    logout( KWorkSpace::ShutdownConfirmNo,
            KWorkSpace::ShutdownTypeHalt );
}

void KDesktop::slotRebootNoCnf()
{
    logout( KWorkSpace::ShutdownConfirmNo,
            KWorkSpace::ShutdownTypeReboot );
}

void KDesktop::setVRoot( bool enable )
{
    if ( enable == set_vroot )
        return;

    set_vroot = enable;
    kDebug(1204) << "setVRoot " << enable << endl;
    KDesktopSettings::setSetVRoot( set_vroot );
    KDesktopSettings::writeConfig();
    slotSetVRoot();
}

void KDesktop::clearCommandHistory()
{
    if ( m_miniCli )
        m_miniCli->clearHistory();
}

void KDesktop::setIconsEnabled( bool enable )
{
    if ( enable == m_bDesktopEnabled )
        return;

    m_bDesktopEnabled = enable;
    kDebug(1204) << "setIcons " << enable << endl;
    KDesktopSettings::setDesktopEnabled( m_bDesktopEnabled );
    KDesktopSettings::writeConfig();
    if (!enable) {
        delete m_pIconView;
        m_pIconView = 0;
    }
    configure();
}

void KDesktop::desktopResized()
{
    resize( kapp->desktop()->size());
}

void KDesktop::switchDesktops( int delta )
{
    bool old = m_bWheelSwitchesWorkspace;
    m_bWheelSwitchesWorkspace = true;
    slotSwitchDesktops(delta);
    m_bWheelSwitchesWorkspace = old;
}

bool KDesktop::event(QEvent * e)
{
    if ( e->type() == QEvent::WindowDeactivate)
    {
        if (m_pIconView)
            m_pIconView->clearSelection();
    }
    return QWidget::event(e);
}

QDBusObjectPath KDesktop::background()
{
    return QDBusObjectPath( KBackgroundManager::backgroundDBusObjectPath );
}

QDBusObjectPath KDesktop::screenSaver()
{
    return QDBusObjectPath( SaverEngine::screenSaverDBusObjectPath );
}

#include "desktop.moc"
