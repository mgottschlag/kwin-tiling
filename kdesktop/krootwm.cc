/*
 * krootwm.cc Part of the KDE project.
 *
 * Copyright (C) 1997 Matthias Ettrich
 *           (C) 1997 Torben Weis, weis@kde.org
 *           (C) 1998 S.u.S.E. weis@suse.de

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License version 2 as published by the Free Software Foundation.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/


#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>

#include <kstandarddirs.h>
#include <kmenu.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kbookmarkmenu.h>
#include <konqbookmarkmanager.h>
#include <klocale.h>
#include <knewmenu.h>
#include <dcopclient.h>
#include <dcopref.h>
#include <khelpmenu.h>
#include <kdebug.h>
#include <kwindowlistmenu.h>
#include <kmenubar.h>
#include <kmessagebox.h>
#include <kuser.h>
#include <ktoolinvocation.h>
#include <qfile.h>

#include "krootwm.h"
#include "kdiconview.h"
#include "desktop.h"
#include "kcustommenu.h"
#include "kdesktopsettings.h"

#include <netwm.h>
#include <X11/X.h>
#include <X11/Xlib.h>

#include <dmctl.h>
#include <QX11Info>
#include <QDesktopWidget>
#include <Q3IconView>
#include <kauthorized.h>
#include <ktoolinvocation.h>

KRootWm * KRootWm::s_rootWm = 0;


// for multihead
extern int kdesktop_screen_number;


KRootWm::KRootWm(KDesktop* _desktop) : QObject(_desktop)
{
  s_rootWm = this;
  m_actionCollection = new KActionCollection(_desktop, this);
  m_pDesktop = _desktop;
  m_bDesktopEnabled = (m_pDesktop->iconView() != 0);
  customMenu1 = 0;
  customMenu2 = 0;


  // Creates the new menu
  menuBar = 0; // no menubar yet
  menuNew = 0;
  if (m_bDesktopEnabled && KAuthorized::authorizeKAction("editable_desktop_icons"))
  {
     menuNew = new KNewMenu( m_actionCollection, "new_menu" );
     connect(menuNew->popupMenu(), SIGNAL( aboutToShow() ),
             this, SLOT( slotFileNewAboutToShow() ) );
     connect( menuNew, SIGNAL( activated() ),
              m_pDesktop->iconView(), SLOT( slotNewMenuActivated() ) );
  }

  if (KAuthorized::authorizeKAction("bookmarks"))
  {
     bookmarks = new KActionMenu( i18n("Bookmarks"), "bookmark", m_actionCollection, "bookmarks" );
     // The KBookmarkMenu is needed to fill the Bookmarks menu in the desktop menubar.
     bookmarkMenu = new KBookmarkMenu( KonqBookmarkManager::self(), new KBookmarkOwner(),
                                    bookmarks->popupMenu(),
                                    m_actionCollection,
                                    true, false );
  }
  else
  {
     bookmarks = 0;
     bookmarkMenu = 0;
  }
  
  // The windowList and desktop menus can be part of a menubar (Mac style)
  // so we create them here
  desktopMenu = new QMenu;
  windowListMenu = new KWindowListMenu;
  connect( windowListMenu, SIGNAL( aboutToShow() ),
           this, SLOT( slotWindowListAboutToShow() ) );

  // Create the actions
#if 0
  if (m_bDesktopEnabled)
  {
      // Don't do that! One action in two parent collections means some invalid write
      // during the second ~KActionCollection.
     KAction *action = m_pDesktop->actionCollection()->action( "paste" );
     if (action)
        m_actionCollection->insert( action );
     action = m_pDesktop->actionCollection()->action( "undo" );
     if (action)
        m_actionCollection->insert( action );
  }
#endif

  if (KAuthorized::authorizeKAction("run_command"))
  {
     new KAction(i18n("Run Command..."), "run", 0, m_pDesktop, SLOT( slotExecuteCommand() ), m_actionCollection, "exec" );
  }
  if (!KGlobal::config()->isImmutable())
  {
     new KAction(i18n("Configure Desktop..."), "configure", 0, this, SLOT( slotConfigureDesktop() ),
                 m_actionCollection, "configdesktop" );
     new KAction(i18n("Disable Desktop Menu"), 0, this, SLOT( slotToggleDesktopMenu() ),
                 m_actionCollection, "togglemenubar" );
  }

  new KAction(i18n("Unclutter Windows"), 0, this, SLOT( slotUnclutterWindows() ),
              m_actionCollection, "unclutter" );
  new KAction(i18n("Cascade Windows"), 0, this, SLOT( slotCascadeWindows() ),
              m_actionCollection, "cascade" );

  // arrange menu actions
  if (m_bDesktopEnabled && KAuthorized::authorizeKAction("editable_desktop_icons"))
  {
     new KAction(i18n("By Name (Case Sensitive)"), 0, this, SLOT( slotArrangeByNameCS() ),
                 m_actionCollection, "sort_ncs");
     new KAction(i18n("By Name (Case Insensitive)"), 0, this, SLOT( slotArrangeByNameCI() ),
                 m_actionCollection, "sort_nci");
     new KAction(i18n("By Size"), 0, this, SLOT( slotArrangeBySize() ),
                 m_actionCollection, "sort_size");
     new KAction(i18n("By Type"), 0, this, SLOT( slotArrangeByType() ),
                 m_actionCollection, "sort_type");
     new KAction(i18n("By Date"), 0, this, SLOT( slotArrangeByDate() ),
                 m_actionCollection, "sort_date");

     KToggleAction *aSortDirsFirst = new KToggleAction( i18n("Directories First"), 0, m_actionCollection, "sort_directoriesfirst" );
     connect( aSortDirsFirst, SIGNAL( toggled( bool ) ),
              this, SLOT( slotToggleDirFirst( bool ) ) );
     new KAction(i18n("Line Up Horizontally"), 0,
                 this, SLOT( slotLineupIconsHoriz() ),
                 m_actionCollection, "lineupHoriz" );
     new KAction(i18n("Line Up Vertically"), 0,
                 this, SLOT( slotLineupIconsVert() ),
                 m_actionCollection, "lineupVert" );
     KToggleAction *aAutoAlign = new KToggleAction(i18n("Align to Grid"), 0,
                 m_actionCollection, "realign" );
     connect( aAutoAlign, SIGNAL( toggled( bool ) ),
              this, SLOT( slotToggleAutoAlign( bool ) ) );
     KToggleAction *aLockIcons = new KToggleAction(i18n("Lock in Place"), 0, m_actionCollection, "lock_icons");
     connect( aLockIcons, SIGNAL( toggled( bool ) ),
              this, SLOT( slotToggleLockIcons( bool ) ) );
  }
  if (m_bDesktopEnabled)
  {
     new KAction(i18n("Refresh Desktop"), "desktop", 0, this, SLOT( slotRefreshDesktop() ),
                 m_actionCollection, "refresh" );
  }
  // Icons in sync with kicker
  if (KAuthorized::authorizeKAction("lock_screen"))
  {
      new KAction(i18n("Lock Session"), "lock", 0, this, SLOT( slotLock() ),
                  m_actionCollection, "lock" );
  }
  if (KAuthorized::authorizeKAction("logout"))
  {
      new KAction(i18n("Log Out \"%1\"...").arg(KUser().loginName()), "exit", 0,
                  this, SLOT( slotLogout() ), m_actionCollection, "logout" );
  }

  if (KAuthorized::authorizeKAction("start_new_session") && DM().isSwitchable())
  {
      new KAction(i18n("Start New Session"), "fork", 0, this,
                  SLOT( slotNewSession() ), m_actionCollection, "newsession" );
      if (KAuthorized::authorizeKAction("lock_screen"))
      {
          new KAction(i18n("Lock Current && Start New Session"), "lock", 0, this,
                      SLOT( slotLockNNewSession() ), m_actionCollection, "lockNnewsession" );
      }
  }

  initConfig();
}

KRootWm::~KRootWm()
{
  delete m_actionCollection;
  delete desktopMenu;
  delete windowListMenu;
}

void KRootWm::initConfig()
{
//  kDebug() << "KRootWm::initConfig" << endl;

  // parse the configuration
  m_bGlobalMenuBar = KDesktopSettings::macStyle();
  m_bShowMenuBar = m_bGlobalMenuBar || KDesktopSettings::showMenubar();

  static const int choiceCount = 7;
  // read configuration for clicks on root window
  static const char * const s_choices[choiceCount] = { "", "WindowListMenu", "DesktopMenu", "AppMenu", "CustomMenu1", "CustomMenu2", "BookmarksMenu" };
  leftButtonChoice = middleButtonChoice = rightButtonChoice = NOTHING;
  QString s = KDesktopSettings::left();
  for ( int c = 0 ; c < choiceCount ; c ++ )
    if (s == s_choices[c])
      { leftButtonChoice = (menuChoice) c; break; }
  s = KDesktopSettings::middle();
  for ( int c = 0 ; c < choiceCount ; c ++ )
    if (s == s_choices[c])
      { middleButtonChoice = (menuChoice) c; break; }
  s = KDesktopSettings::right();
  for ( int c = 0 ; c < choiceCount ; c ++ )
    if (s == s_choices[c])
      { rightButtonChoice = (menuChoice) c; break; }

  // Read configuration for icons alignment
  if ( m_bDesktopEnabled ) {
    m_pDesktop->iconView()->setAutoAlign( KDesktopSettings::autoLineUpIcons() );
    if ( KAuthorized::authorizeKAction( "editable_desktop_icons" ) ) {
        m_pDesktop->iconView()->setItemsMovable( !KDesktopSettings::lockIcons() );
        KToggleAction *aLockIcons = static_cast<KToggleAction*>(m_actionCollection->action("lock_icons"));
        if (aLockIcons)
            aLockIcons->setChecked( KDesktopSettings::lockIcons()  );
    }
    KToggleAction *aAutoAlign = static_cast<KToggleAction*>(m_actionCollection->action("realign"));
    if (aAutoAlign)
        aAutoAlign->setChecked( KDesktopSettings::autoLineUpIcons() );
    KToggleAction *aSortDirsFirst = static_cast<KToggleAction*>(m_actionCollection->action("sort_directoriesfirst"));
    if (aSortDirsFirst)
        aSortDirsFirst->setChecked( KDesktopSettings::sortDirectoriesFirst() );
  }

  buildMenus();
}

void KRootWm::buildMenus()
{
//    kDebug() << "KRootWm::buildMenus" << endl;

    delete menuBar;
    menuBar = 0;

    delete customMenu1;
    customMenu1 = 0;
    delete customMenu2;
    customMenu2 = 0;

    if (m_bShowMenuBar)
    {
//        kDebug() << "showMenuBar" << endl;
        menuBar = new KMenuBar;
        menuBar->setWindowTitle("KDE Desktop");
    }

    // create Arrange menu
    QMenu *pArrangeMenu = 0;
    QMenu *pLineupMenu = 0;
    KAction *action;
    help = new KHelpMenu(0, 0, false);
    // help->menu()->removeItem( KHelpMenu::menuAboutApp );

    if (m_bDesktopEnabled && m_actionCollection->action("realign"))
    {
        pArrangeMenu = new QMenu;
        m_actionCollection->action("sort_ncs")->plug( pArrangeMenu );
        m_actionCollection->action("sort_nci")->plug( pArrangeMenu );
        m_actionCollection->action("sort_size")->plug( pArrangeMenu );
        m_actionCollection->action("sort_type")->plug( pArrangeMenu );
        m_actionCollection->action("sort_date" )->plug( pArrangeMenu );
        pArrangeMenu->addSeparator();
        m_actionCollection->action("sort_directoriesfirst")->plug( pArrangeMenu );

        pLineupMenu = new QMenu;
        m_actionCollection->action( "lineupHoriz" )->plug( pLineupMenu );
        m_actionCollection->action( "lineupVert" )->plug( pLineupMenu );
        pLineupMenu->addSeparator();
        m_actionCollection->action( "realign" )->plug( pLineupMenu );
    }

    sessionsMenu = 0;
    if (m_actionCollection->action("newsession"))
    {
        sessionsMenu = new QMenu;
        connect( sessionsMenu, SIGNAL(aboutToShow()), SLOT(slotPopulateSessions()) );
        connect( sessionsMenu, SIGNAL(activated(int)), SLOT(slotSessionActivated(int)) );
    }

    if (menuBar) {
        bool needSeparator = false;
        file = new QMenu;

        action = m_actionCollection->action("exec");
        if (action)
        {
            action->plug( file );
            file->addSeparator();
        }

        action = m_actionCollection->action("lock");
        if (action)
            action->plug( file );

        action = m_actionCollection->action("logout");
        if (action)
            action->plug( file );

        desk = new QMenu;

        if (m_bDesktopEnabled)
        {
            m_actionCollection->action("unclutter")->plug( desk );
            m_actionCollection->action("cascade")->plug( desk );
            desk->addSeparator();

            if (pArrangeMenu)
                desk->insertItem(i18n("Sort Icons"), pArrangeMenu);
            if (pLineupMenu)
                desk->insertItem(i18n("Line Up Icons"), pLineupMenu );

            m_actionCollection->action("refresh")->plug( desk );
            needSeparator = true;
        }
        action = m_actionCollection->action("configdesktop");
        if (action)
        {
           if (needSeparator)
              desk->addSeparator();
           action->plug( desk );
           needSeparator = true;
        }

        action = m_actionCollection->action("togglemenubar");
        if (action)
        {
           if (needSeparator)
              desk->addSeparator();
           action->plug( desk );
           action->setText(i18n("Disable Desktop Menu"));
        }
    }
    else
    {
        action = m_actionCollection->action("togglemenubar");
        if (action)
           action->setText(i18n("Enable Desktop Menu"));
    }

    desktopMenu->clear();
    desktopMenu->disconnect( this );
    bool needSeparator = false;

    if (menuNew)
    {
       menuNew->plug( desktopMenu );
       needSeparator = true;
    }

#if 0
    if (bookmarks)
    {
       bookmarks->plug( desktopMenu );
       needSeparator = true;
    }
#endif

    action = m_actionCollection->action("exec");
    if (action)
    {
       action->plug( desktopMenu );
       needSeparator = true;
    }

    if (needSeparator)
    {
        desktopMenu->addSeparator();
        needSeparator = false;
    }

    if (m_bDesktopEnabled)
    {
        action = m_pDesktop->actionCollection()->action( "undo" );
        if (action)
           action->plug( desktopMenu );
        action = m_pDesktop->actionCollection()->action( "paste" );
        if (action)
           action->plug( desktopMenu );
        desktopMenu->addSeparator();
    }

    if (m_bDesktopEnabled && m_actionCollection->action("realign"))
    {
       QMenu* pIconOperationsMenu = new QMenu;

       pIconOperationsMenu->insertItem(i18n("Sort Icons"), pArrangeMenu);
       pIconOperationsMenu->addSeparator();
       m_actionCollection->action( "lineupHoriz" )->plug( pIconOperationsMenu );
       m_actionCollection->action( "lineupVert" )->plug( pIconOperationsMenu );
       pIconOperationsMenu->addSeparator();
       m_actionCollection->action( "realign" )->plug( pIconOperationsMenu );
       KAction *aLockIcons = m_actionCollection->action( "lock_icons" );
       if ( aLockIcons )
           aLockIcons->plug( pIconOperationsMenu );

       desktopMenu->insertItem(SmallIconSet("icons"), i18n("Icons"), pIconOperationsMenu);
    }

    QMenu* pWindowOperationsMenu = new QMenu;
    m_actionCollection->action("cascade")->plug( pWindowOperationsMenu );
    m_actionCollection->action("unclutter")->plug( pWindowOperationsMenu );
    desktopMenu->insertItem(SmallIconSet("window_list"), i18n("Windows"), pWindowOperationsMenu);

    if (m_bDesktopEnabled)
    {
       m_actionCollection->action("refresh")->plug( desktopMenu );
    }

    action = m_actionCollection->action("configdesktop");
    if (action)
    {
       action->plug( desktopMenu );
    }
    int lastSep = desktopMenu->insertSeparator();

    if (sessionsMenu && KAuthorized::authorizeKAction("switch_user"))
    {
        desktopMenu->insertItem(SmallIconSet("switchuser" ), i18n("Switch User"), sessionsMenu);
        needSeparator = true;
    }

    action = m_actionCollection->action("lock");
    if (action)
    {
        action->plug( desktopMenu );
        needSeparator = true;
    }

    action = m_actionCollection->action("logout");
    if (action)
    {
        action->plug( desktopMenu );
        needSeparator = true;
    }

    if (!needSeparator)
    {
        desktopMenu->removeItem(lastSep);
    }

    connect( desktopMenu, SIGNAL( aboutToShow() ), this, SLOT( slotFileNewAboutToShow() ) );

    if (menuBar) {
        menuBar->insertItem(i18n("File"), file);
        if (sessionsMenu)
        {
            menuBar->insertItem(i18n("Sessions"), sessionsMenu);
        }
        if (menuNew)
        {
            menuBar->insertItem(i18n("New"), menuNew->popupMenu());
        }
        if (bookmarks)
        {
            menuBar->insertItem(i18n("Bookmarks"), bookmarks->popupMenu());
        }
        menuBar->insertItem(i18n("Desktop"), desk);
        menuBar->insertItem(i18n("Windows"), windowListMenu);
        menuBar->insertItem(i18n("Help"), help->menu());

        menuBar->setTopLevelMenu( true );
        menuBar->show(); // we need to call show() as we delayed the creation with the timer
    }
}

void KRootWm::slotToggleDirFirst( bool b )
{
    KDesktopSettings::setSortDirectoriesFirst( b );
    KDesktopSettings::writeConfig();
}

void KRootWm::slotToggleAutoAlign( bool b )
{
    KDesktopSettings::setAutoLineUpIcons( b );
    KDesktopSettings::writeConfig();

    // Auto line-up icons
    m_pDesktop->iconView()->setAutoAlign( b );
}

void KRootWm::slotFileNewAboutToShow()
{
  if (menuNew)
  {
//  kDebug() << " KRootWm:: (" << this << ") slotFileNewAboutToShow() menuNew=" << menuNew << endl;
     // As requested by KNewMenu :
     menuNew->slotCheckUpToDate();
     // And set the files that the menu apply on :
     menuNew->setPopupFiles( m_pDesktop->url() );
  }
}

void KRootWm::slotWindowListAboutToShow()
{
  windowListMenu->init();
}

void KRootWm::activateMenu( menuChoice choice, const QPoint& global )
{
  switch ( choice )
  {
    case SESSIONSMENU:
      if (sessionsMenu)
          sessionsMenu->popup(global);
      break;
    case WINDOWLISTMENU:
      windowListMenu->popup(global);
      break;
    case DESKTOPMENU:
      m_desktopMenuPosition = global; // for KDIconView::slotPaste
      desktopMenu->popup(global);
      break;
    case BOOKMARKSMENU:
      if (bookmarks)
        bookmarks->popup(global);
      break;
    case APPMENU:
    {
      // This allows the menu to disappear when clicking on the background another time
      XUngrabPointer(QX11Info::display(), CurrentTime);
      XSync(QX11Info::display(), False);
      // Ask kicker to showup the menu
      // make sure we send the message to the correct kicker
      QString appname;
      if (kdesktop_screen_number == 0)
	  appname = QLatin1String("kicker");
      else
	  appname.sprintf("kicker-screen-%d", kdesktop_screen_number);
	  
      DCOPCString name = appname.toLatin1();

      DCOPRef( name, name ).send( "popupKMenu", global );
      break;
    }
    case CUSTOMMENU1:
      if (!customMenu1)
         customMenu1 = new KCustomMenu("kdesktop_custom_menu1");
      customMenu1->popup(global);
      break;
    case CUSTOMMENU2:
      if (!customMenu2)
         customMenu2 = new KCustomMenu("kdesktop_custom_menu2");
      customMenu2->popup(global);
      break;
    case NOTHING:
    default:
      break;
  }
}

void KRootWm::mousePressed( const QPoint& _global, int _button )
{
    if (!desktopMenu) return; // initialisation not yet done
    switch ( _button ) {
    case Qt::LeftButton:
        if ( m_bShowMenuBar && menuBar )
            menuBar->raise();
        activateMenu( leftButtonChoice, _global );
        break;
    case Qt::MidButton:
        activateMenu( middleButtonChoice, _global );
        break;
    case Qt::RightButton:
        if (!KAuthorized::authorizeKAction("action/kdesktop_rmb")) return;
        activateMenu( rightButtonChoice, _global );
        break;
    default:
        // nothing
        break;
    }
}

void KRootWm::slotWindowList() {
//  kDebug() << "KRootWm::slotWindowList" << endl;
// Popup at the center of the screen, this is from keyboard shortcut.
  QDesktopWidget* desktop = KApplication::desktop();
  QRect r = desktop->screenGeometry( desktop->screenNumber(QCursor::pos()));
  windowListMenu->init();
  disconnect( windowListMenu, SIGNAL( aboutToShow() ),
           this, SLOT( slotWindowListAboutToShow() ) ); // avoid calling init() twice
  // windowListMenu->rect() is not valid before showing, use sizeHint()
  windowListMenu->popup(r.center() - QRect( QPoint( 0, 0 ), windowListMenu->sizeHint()).center());
  windowListMenu->selectActiveWindow(); // make the popup more useful
  connect( windowListMenu, SIGNAL( aboutToShow() ),
           this, SLOT( slotWindowListAboutToShow() ) );
}

void KRootWm::slotSwitchUser() {
//  kDebug() << "KRootWm::slotSwitchUser" << endl;
  if (!sessionsMenu)
    return;
  QDesktopWidget* desktop = KApplication::desktop();
  QRect r = desktop->screenGeometry( desktop->screenNumber(QCursor::pos()));
  slotPopulateSessions();
  disconnect( sessionsMenu, SIGNAL( aboutToShow() ),
           this, SLOT( slotPopulateSessions() ) ); // avoid calling init() twice
  sessionsMenu->popup(r.center() - QRect( QPoint( 0, 0 ), sessionsMenu->sizeHint()).center());
  connect( sessionsMenu, SIGNAL( aboutToShow() ),
           SLOT( slotPopulateSessions() ) );
}

void KRootWm::slotArrangeByNameCS()
{
    if (m_bDesktopEnabled)
    {
        bool b = static_cast<KToggleAction *>(m_actionCollection->action("sort_directoriesfirst"))->isChecked();
        m_pDesktop->iconView()->rearrangeIcons( KDIconView::NameCaseSensitive, b);
    }
}

void KRootWm::slotArrangeByNameCI()
{
    if (m_bDesktopEnabled)
    {
        bool b = static_cast<KToggleAction *>(m_actionCollection->action("sort_directoriesfirst"))->isChecked();
        m_pDesktop->iconView()->rearrangeIcons( KDIconView::NameCaseInsensitive, b);
    }
}

void KRootWm::slotArrangeBySize()
{
    if (m_bDesktopEnabled)
    {
        bool b = static_cast<KToggleAction *>(m_actionCollection->action("sort_directoriesfirst"))->isChecked();
        m_pDesktop->iconView()->rearrangeIcons( KDIconView::Size, b);
    }
}

void KRootWm::slotArrangeByDate()
{
    if (m_bDesktopEnabled)
    {
        bool b = static_cast<KToggleAction *>( m_actionCollection->action( "sort_directoriesfirst" ) )->isChecked();
        m_pDesktop->iconView()->rearrangeIcons( KDIconView::Date, b );
    }
}

void KRootWm::slotArrangeByType()
{
    if (m_bDesktopEnabled)
    {
        bool b = static_cast<KToggleAction *>(m_actionCollection->action("sort_directoriesfirst"))->isChecked();
        m_pDesktop->iconView()->rearrangeIcons( KDIconView::Type, b);
    }
}

void KRootWm::slotLineupIconsHoriz() {
    if (m_bDesktopEnabled)
    {
        m_pDesktop->iconView()->lineupIcons(Q3IconView::LeftToRight);
    }
}

void KRootWm::slotLineupIconsVert()  {
    if (m_bDesktopEnabled)
    {
        m_pDesktop->iconView()->lineupIcons(Q3IconView::TopToBottom);
    }
}

void KRootWm::slotLineupIcons() {
    if (m_bDesktopEnabled)
    {
        m_pDesktop->iconView()->lineupIcons();
    }
}

void KRootWm::slotToggleLockIcons( bool lock )
{
    if (m_bDesktopEnabled)
    {
        m_pDesktop->iconView()->setItemsMovable( !lock );
        KDesktopSettings::setLockIcons( lock );
        KDesktopSettings::writeConfig();
    }
}

void KRootWm::slotRefreshDesktop() {
    if (m_bDesktopEnabled)
    {
        m_pDesktop->refresh();
    }
}

QStringList KRootWm::configModules() {
  QStringList args;
  args << "kde-background.desktop" << "kde-desktopbehavior.desktop"  << "kde-desktop.desktop"
	  << "kde-screensaver.desktop" << "kde-display.desktop";
  return args;
}

void KRootWm::slotConfigureDesktop() {
  QStringList args = configModules();
  args.prepend(i18n("Desktop"));
  args.prepend("--caption");
  KToolInvocation::kdeinitExec(QLatin1String("kcmshell"), args);
}

static void sendToAppropriate(const char* baseApp, const char* iface, const char* call)
{
    // make sure we send the message to the correct screen
    QString appname;
    if (kdesktop_screen_number == 0)
	appname = QLatin1String(baseApp);
    else
	appname.sprintf("%s-screen-%d", baseApp, kdesktop_screen_number);

    QByteArray data;
    kapp->dcopClient()->send( DCOPCString(appname.toLatin1()), DCOPCString(iface), DCOPCString(call), QByteArray());
}

void KRootWm::slotToggleDesktopMenu()
{
    KDesktopSettings::setShowMenubar( !(m_bShowMenuBar && menuBar) );
    KDesktopSettings::writeConfig();

    sendToAppropriate("kdesktop", "KDesktopIface", "configure()");
    // for the standalone menubar setting
    kapp->dcopClient()->send( "menuapplet*", "menuapplet", "configure()", QByteArray() );
    kapp->dcopClient()->send( "kicker", "kicker", "configureMenubar()", QByteArray() );
    kapp->dcopClient()->send( "kwin*", "", "reconfigure()", QByteArray() );
}


void KRootWm::slotUnclutterWindows() {
    sendToAppropriate("kwin", "KWinInterface", "unclutterDesktop()");
}


void KRootWm::slotCascadeWindows() {
    sendToAppropriate("kwin", "KWinInterface", "cascadeDesktop()");
}


void KRootWm::slotLock() {
    sendToAppropriate("kdesktop", "KScreensaverInterface", "lock()");
}


void KRootWm::slotLogout() {
    m_pDesktop->logout(KWorkSpace::ShutdownConfirmDefault, KWorkSpace::ShutdownTypeDefault);
}

void KRootWm::slotPopulateSessions()
{
    KAction *action;
    int p;
    DM dm;

    sessionsMenu->clear();
    action = m_actionCollection->action("newsession");
    if (action && (p = dm.numReserve()) >= 0)
    {
        action->plug( sessionsMenu );
        action->setEnabled( p );
        action = m_actionCollection->action("lockNnewsession");
        if (action)
        {
            action->plug( sessionsMenu );
            action->setEnabled( p );
        }
        sessionsMenu->addSeparator();
    }
    SessList sess;
    if (dm.localSessions( sess ))
        for (SessList::ConstIterator it = sess.begin(); it != sess.end(); ++it) {
            int id = sessionsMenu->insertItem( DM::sess2Str( *it ), (*it).vt );
            if (!(*it).vt)
                sessionsMenu->setItemEnabled( id, false );
            if ((*it).self)
                sessionsMenu->setItemChecked( id, true );
        }
}

void KRootWm::slotSessionActivated( int ent )
{
    if (ent > 0 && !sessionsMenu->isItemChecked( ent ))
        DM().lockSwitchVT( ent );
}

void KRootWm::slotNewSession()
{
    doNewSession( false );
}

void KRootWm::slotLockNNewSession()
{
    doNewSession( true );
}

void KRootWm::doNewSession( bool lock )
{
    int result = KMessageBox::warningContinueCancel(
        m_pDesktop,
        i18n("<p>You have chosen to open another desktop session.<br>"
               "The current session will be hidden "
               "and a new login screen will be displayed.<br>"
               "An F-key is assigned to each session; "
               "F%1 is usually assigned to the first session, "
               "F%2 to the second session and so on. "
               "You can switch between sessions by pressing "
               "Ctrl, Alt and the appropriate F-key at the same time. "
               "Additionally, the KDE Panel and Desktop menus have "
               "actions for switching between sessions.</p>")
                           .arg(7).arg(8),
        i18n("Warning - New Session"),
        KGuiItem(i18n("&Start New Session"), "fork"),
        ":confirmNewSession",
        KMessageBox::PlainCaption | KMessageBox::Notify);

    if (result==KMessageBox::Cancel)
        return;

    if (lock)
        slotLock();

    DM().startReserve();
}

void KRootWm::slotMenuItemActivated(int /* item */ )
{
}

#include "krootwm.moc"
