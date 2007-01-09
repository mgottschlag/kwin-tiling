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

#include <kactioncollection.h>
#include <kapplication.h>
#include <kbookmarkmenu.h>
#include <kconfig.h>
#include <kdebug.h>
#include <khelpmenu.h>
#include <kstandarddirs.h>
#include <kicon.h>
#include <klocale.h>
#include <kmenu.h>
#include <kmenubar.h>
#include <kmessagebox.h>
#include <knewmenu.h>
#include <konqbookmarkmanager.h>
#include <ktoggleaction.h>
#include <ktoolinvocation.h>
#include <kuser.h>
#include <kwindowlistmenu.h>

#include <QFile>

#include "krootwm.h"
#include "kdiconview.h"
#include "desktop.h"
#include "kcustommenu.h"
#include "kdesktopsettings.h"

#include <QtDBus/QtDBus>

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
  m_actionCollection = new KActionCollection(this);
  m_actionCollection->setAssociatedWidget(_desktop);
  m_pDesktop = _desktop;
  m_bDesktopEnabled = (m_pDesktop->iconView() != 0);
  customMenu1 = 0;
  customMenu2 = 0;


  // Creates the new menu
  menuBar = 0; // no menubar yet
  menuNew = 0;
  if (m_bDesktopEnabled && KAuthorized::authorizeKAction("editable_desktop_icons"))
  {
     menuNew = new KNewMenu( m_actionCollection, m_pDesktop, "new_menu" );
     connect(menuNew->menu(), SIGNAL( aboutToShow() ),
             this, SLOT( slotFileNewAboutToShow() ) );
     connect( menuNew, SIGNAL( activated() ),
              m_pDesktop->iconView(), SLOT( slotNewMenuActivated() ) );
  }

  if (KAuthorized::authorizeKAction("bookmarks"))
  {
     bookmarks = new KActionMenu( KIcon("bookmark"), i18n("Bookmarks"), this );
     m_actionCollection->addAction( "bookmarks", bookmarks );
     // The KBookmarkMenu is needed to fill the Bookmarks menu in the desktop menubar.
     bookmarkMenu = new KBookmarkMenu( KonqBookmarkManager::self(), 0,
                                       bookmarks->menu(),
                                       m_actionCollection );
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
     QAction *action = m_pDesktop->actionCollection()->action( "paste" );
     if (action)
        m_actionCollection->insert( action );
     action = m_pDesktop->actionCollection()->action( "undo" );
     if (action)
        m_actionCollection->insert( action );
  }
#endif

  QAction *action;

  if (KAuthorized::authorizeKAction("run_command"))
  {
      action = m_actionCollection->addAction( "exec" );
      action->setIcon( KIcon("run") );
      action->setText( i18n("Run Command...") );
      connect(action, SIGNAL(triggered(bool)), m_pDesktop, SLOT( slotExecuteCommand() ));
  }
  if (!KGlobal::config()->isImmutable())
  {
      action = m_actionCollection->addAction( "configdesktop" );
      action->setIcon( KIcon("configure") );
      action->setText( i18n("Configure Desktop...") );
      connect(action, SIGNAL(triggered(bool)), SLOT( slotConfigureDesktop() ));
      action = m_actionCollection->addAction( "togglemenubar" );
      action->setText( i18n("Disable Desktop Menu") );
      connect(action, SIGNAL(triggered(bool) ), SLOT( slotToggleDesktopMenu() ));
  }

  action = m_actionCollection->addAction( "unclutter" );
  action->setText( i18n("Unclutter Windows") );
  connect(action, SIGNAL(triggered(bool) ), SLOT( slotUnclutterWindows() ));
  action = m_actionCollection->addAction( "cascade" );
  action->setText( i18n("Cascade Windows") );
  connect(action, SIGNAL(triggered(bool) ), SLOT( slotCascadeWindows() ));

  // arrange menu actions
  if (m_bDesktopEnabled && KAuthorized::authorizeKAction("editable_desktop_icons"))
  {
     action = m_actionCollection->addAction( "sort_ncs" );
     action->setText( i18n("By Name (Case Sensitive)") );
     connect(action, SIGNAL(triggered(bool) ), SLOT( slotArrangeByNameCS() ));
     action = m_actionCollection->addAction( "sort_nci" );
     action->setText( i18n("By Name (Case Insensitive)") );
     connect(action, SIGNAL(triggered(bool) ), SLOT( slotArrangeByNameCI() ));
     action = m_actionCollection->addAction( "sort_size" );
     action->setText( i18n("By Size") );
     connect(action, SIGNAL(triggered(bool) ), SLOT( slotArrangeBySize() ));
     action = m_actionCollection->addAction( "sort_type" );
     action->setText( i18n("By Type") );
     connect(action, SIGNAL(triggered(bool) ), SLOT( slotArrangeByType() ));
     action = m_actionCollection->addAction( "sort_date" );
     action->setText( i18n("By Date") );
     connect(action, SIGNAL(triggered(bool) ), SLOT( slotArrangeByDate() ));

     KToggleAction *aSortDirsFirst = new KToggleAction( i18n("Directories First"), this );
     m_actionCollection->addAction( "sort_directoriesfirst", aSortDirsFirst );
     connect( aSortDirsFirst, SIGNAL( toggled( bool ) ),
              this, SLOT( slotToggleDirFirst( bool ) ) );
     action = m_actionCollection->addAction( "lineupHoriz" );
     action->setText( i18n("Line Up Horizontally") );
     connect(action, SIGNAL(triggered(bool) ), SLOT( slotLineupIconsHoriz() ));
     action = m_actionCollection->addAction( "lineupVert" );
     action->setText( i18n("Line Up Vertically") );
     connect(action, SIGNAL(triggered(bool) ), SLOT( slotLineupIconsVert() ));
     KToggleAction *aAutoAlign = new KToggleAction(i18n("Align to Grid"), this );
     m_actionCollection->addAction( "realign", aAutoAlign );
     connect( aAutoAlign, SIGNAL( toggled( bool ) ),
              this, SLOT( slotToggleAutoAlign( bool ) ) );
     KToggleAction *aLockIcons = new KToggleAction(i18n("Lock in Place"), this);
     m_actionCollection->addAction( "lock_icons", aLockIcons );
     connect( aLockIcons, SIGNAL( toggled( bool ) ),
              this, SLOT( slotToggleLockIcons( bool ) ) );
  }
  if (m_bDesktopEnabled)
  {
     QAction *action = m_actionCollection->addAction( "refresh" );
     action->setIcon( KIcon("desktop") );
     action->setText( i18n("Refresh Desktop") );
     connect(action, SIGNAL(triggered(bool)), SLOT( slotRefreshDesktop() ));
  }
  // Icons in sync with kicker
  if (KAuthorized::authorizeKAction("lock_screen"))
  {
      QAction *action = m_actionCollection->addAction( "lock" );
      action->setIcon( KIcon("lock") );
      action->setText( i18n("Lock Session") );
      connect(action, SIGNAL(triggered(bool)), SLOT( slotLock() ));
  }
  if (KAuthorized::authorizeKAction("logout"))
  {
      QAction *action = m_actionCollection->addAction( "logout" );
      action->setIcon( KIcon("exit") );
      action->setText( i18n("Log Out \"%1\"...", KUser().loginName()) );
      connect(action, SIGNAL(triggered(bool)), SLOT( slotLogout() ));

  }

  if (KAuthorized::authorizeKAction("start_new_session") && DM().isSwitchable())
  {
      QAction *action = m_actionCollection->addAction( "newsession" );
      action->setIcon( KIcon("fork") );
      action->setText( i18n("Start New Session") );
      connect(action, SIGNAL(triggered(bool)), SLOT( slotNewSession() ));
      if (KAuthorized::authorizeKAction("lock_screen"))
      {
          QAction *action = m_actionCollection->addAction( "lockNnewsession" );
          action->setIcon( KIcon("lock") );
          action->setText( i18n("Lock Current && Start New Session") );
          connect(action, SIGNAL(triggered(bool)), SLOT( slotLockNNewSession() ));
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
    QAction *action;
    help = new KHelpMenu(0, 0, false);
    // help->menu()->removeItem( KHelpMenu::menuAboutApp );

    if (m_bDesktopEnabled && m_actionCollection->action("realign"))
    {
        pArrangeMenu = new QMenu;
        pArrangeMenu->addAction( m_actionCollection->action("sort_ncs") );
        pArrangeMenu->addAction( m_actionCollection->action("sort_nci") );
        pArrangeMenu->addAction( m_actionCollection->action("sort_size") );
        pArrangeMenu->addAction( m_actionCollection->action("sort_type") );
	pArrangeMenu->addAction( m_actionCollection->action("sort_date" ) );
        pArrangeMenu->addSeparator();
        pArrangeMenu->addAction( m_actionCollection->action("sort_directoriesfirst") );

        pLineupMenu = new QMenu;
        pLineupMenu->addAction( m_actionCollection->action( "lineupHoriz" ) );
	pLineupMenu->addAction( m_actionCollection->action( "lineupVert" ) );
        pLineupMenu->addSeparator();
	pLineupMenu->addAction( m_actionCollection->action( "realign" ) );
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
            file->addAction( action );
            file->addSeparator();
        }

        action = m_actionCollection->action("lock");
        if (action)
            file->addAction( action );

        action = m_actionCollection->action("logout");
        if (action)
            file->addAction( action );

        desk = new QMenu;

        if (m_bDesktopEnabled)
        {
            desk->addAction( m_actionCollection->action("unclutter") );
            desk->addAction( m_actionCollection->action("cascade") );
            desk->addSeparator();

            if (pArrangeMenu)
                desk->insertItem(i18n("Sort Icons"), pArrangeMenu);
            if (pLineupMenu)
                desk->insertItem(i18n("Line Up Icons"), pLineupMenu );

            desk->addAction( m_actionCollection->action("refresh") );
            needSeparator = true;
        }
        action = m_actionCollection->action("configdesktop");
        if (action)
        {
           if (needSeparator)
              desk->addSeparator();
           desk->addAction( action );
           needSeparator = true;
        }

        action = m_actionCollection->action("togglemenubar");
        if (action)
        {
           if (needSeparator)
              desk->addSeparator();
           desk->addAction( action );
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
       desktopMenu->addAction( menuNew );
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
       desktopMenu->addAction( action );
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
           desktopMenu->addAction( action );
        action = m_pDesktop->actionCollection()->action( "paste" );
        if (action)
           desktopMenu->addAction( action );
        desktopMenu->addSeparator();
    }

    if (m_bDesktopEnabled && m_actionCollection->action("realign"))
    {
       QMenu* pIconOperationsMenu = new QMenu;

       pIconOperationsMenu->insertItem(i18n("Sort Icons"), pArrangeMenu);
       pIconOperationsMenu->addSeparator();
       pIconOperationsMenu->addAction( m_actionCollection->action( "lineupHoriz" ) );
       pIconOperationsMenu->addAction( m_actionCollection->action( "lineupVert" ) );
       pIconOperationsMenu->addSeparator();
       pIconOperationsMenu->addAction( m_actionCollection->action( "realign" ) );
       QAction *aLockIcons = m_actionCollection->action( "lock_icons" );
       if ( aLockIcons )
           pIconOperationsMenu->addAction( aLockIcons );

       desktopMenu->insertItem(KIcon("icons"), i18n("Icons"), pIconOperationsMenu);
    }

    QMenu* pWindowOperationsMenu = new QMenu;
    pWindowOperationsMenu->addAction( m_actionCollection->action("cascade") );
    pWindowOperationsMenu->addAction( m_actionCollection->action("unclutter") );
    desktopMenu->insertItem(KIcon("window_list"), i18n("Windows"), pWindowOperationsMenu);

    if (m_bDesktopEnabled)
    {
       desktopMenu->addAction( m_actionCollection->action("refresh") );
    }

    action = m_actionCollection->action("configdesktop");
    if (action)
    {
       desktopMenu->addAction( action );
    }
    int lastSep = desktopMenu->insertSeparator();

    if (sessionsMenu && KAuthorized::authorizeKAction("switch_user"))
    {
        desktopMenu->insertItem(KIcon("switchuser" ), i18n("Switch User"), sessionsMenu);
        needSeparator = true;
    }

    action = m_actionCollection->action("lock");
    if (action)
    {
        desktopMenu->addAction( action );
        needSeparator = true;
    }

    action = m_actionCollection->action("logout");
    if (action)
    {
        desktopMenu->addAction( action );
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
            menuBar->insertItem(i18n("New"), menuNew->menu());
        }
        if (bookmarks)
        {
            menuBar->insertItem(i18n("Bookmarks"), bookmarks->menu());
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
        bookmarks->menu()->exec(global);
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

      QDBusInterface kicker( appname, "/Kicker", "org.kde.Kicker" ); // ### check this
      if ( kicker.isValid() )
          kicker.call( "popupKMenu", global );
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
  args << "background" << "desktopbehavior"  << "desktop"
       << "screensaver" << "display";
  return args;
}

void KRootWm::slotConfigureDesktop() {
  QStringList args = configModules();
  args.prepend(i18n("Desktop"));
  args.prepend("--caption");
  KToolInvocation::kdeinitExec(QLatin1String("kcmshell"), args);
}

static void sendToAppropriate(const char* baseApp, const char* iface, const char* call, const char* path)
{
    // make sure we send the message to the correct screen
    QString appname;
    if (kdesktop_screen_number == 0)
	appname = QLatin1String(baseApp);
    else
	appname.sprintf("%s-screen-%d", baseApp, kdesktop_screen_number);

    appname = "org.kde." + appname;
    QDBusInterface interface( appname, path, iface );
    if ( interface.isValid() )
        interface.call( call );
}

void KRootWm::slotToggleDesktopMenu()
{
    KDesktopSettings::setShowMenubar( !(m_bShowMenuBar && menuBar) );
    KDesktopSettings::writeConfig();

    sendToAppropriate("kdesktop", "org.kde.kdesktop.Desktop", "configure", "/Desktop");
    // for the standalone menubar setting
#ifdef __GNUC__
#warning TODO port to a dbus signal
#endif
    // dbus signal should be emitted by kdesktop, and all that code can just connect to it.
#if 0
    kapp->dcopClient()->send( "menuapplet*", "menuapplet", "configure" );
    kapp->dcopClient()->send( "kicker", "kicker", "configureMenubar" );
#endif
    // Send signal to all kwin instances
    QDBusMessage message =
        QDBusMessage::createSignal("/KWin", "org.kde.KWin", "reloadConfig");
    QDBusConnection::sessionBus().send(message);

}


void KRootWm::slotUnclutterWindows() {
    sendToAppropriate("kwin", "org.kde.KWin", "unclutterDesktop", "/KWin");
}


void KRootWm::slotCascadeWindows() {
    sendToAppropriate("kwin", "org.kde.KWin", "cascadeDesktop", "/KWin");
}


void KRootWm::slotLock() {
    sendToAppropriate("kdesktop", "org.kde.kdesktop.ScreenSaver", "lock", "/ScreenSaver" );
}


void KRootWm::slotLogout() {
    m_pDesktop->logout(KWorkSpace::ShutdownConfirmDefault, KWorkSpace::ShutdownTypeDefault);
}

void KRootWm::slotPopulateSessions()
{
    QAction *action;
    int p;
    DM dm;

    sessionsMenu->clear();
    action = m_actionCollection->action("newsession");
    if (action && (p = dm.numReserve()) >= 0)
    {
        sessionsMenu->addAction( action );
        action->setEnabled( p );
        action = m_actionCollection->action("lockNnewsession");
        if (action)
        {
            sessionsMenu->addAction( action );
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
               "actions for switching between sessions.</p>",
                            7, 8),
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
